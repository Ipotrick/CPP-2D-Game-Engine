#include "SpritePipe.hpp"

#define SPRITE_VERTEX_ATTRIBUTES \
	Vec2, \
	Vec2

#define SPRITE_INSTANCE_VERTEX_ATTRIBUTES \
	Vec4,\
	Vec4,\
	Vec2,\
	Vec2,\
	Vec2,\
	Vec2,\
	Vec2,\
	Vec2,\
	GLint,\
	GLint,\
	float,\
	GLint

#include "../TextureSamplerManager.hpp"


SpritePipe::SpritePipe(TextureManager::Backend* texBackend) : texBackend{ texBackend }
{
	texLock.unlock();

	blendingStack.push(gl::BlendingFunction{});
	depthTestStack.push(gl::DepthTest::LessEqual);
	mtsdfSmoothingStack.push(1.25f);
}

void SpritePipe::init(RenderPipeContext& context)
{
	bInUse = true;

	// querry driver for hardware info:
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);

	initializeSpriteShader();
	bInUse = false;
}

void SpritePipe::reset(RenderPipeContext& context)
{
	bInUse = true;
	texLock.lock();
	texBackend->unloadQueue(texLock);
	texLock.unlock();
	bInUse = false;
}

void SpritePipe::render(RenderPipeContext& context)
{
	bInUse = true;
	texLock.lock();
	texBackend->unloadQueue(texLock);
	texBackend->createAndLoadQueueExecute(texLock);

	trisDrawn = 0;

	auto& camera = context.camera;
	const f32 uniformXBend = cast<f32>(context.mainFrameBuffer.getSize().second) / cast<f32>(context.mainFrameBuffer.getSize().first);
	Mat4 worldViewProjMat =
		Mat4::scale({ uniformXBend, 1.0f, 1.0f }) *
		Mat4::scale(camera.zoom) *
		Mat4::scale({ camera.frustumBend.x, camera.frustumBend.y, 1.0f }) *
		Mat4::rotate_z(-camera.rotation) *
		Mat4::translate({ -camera.position.x, -camera.position.y, 0.0f });
	Mat4 pixelViewProjMatrix =
		Mat4::translate(Vec3(-1, -1, 0)) *
		Mat4::scale(Vec3(1.0f / (context.windowWidth), 1.0f / (context.windowHeight), 1.0f)) *
		Mat4::scale(Vec3(2.0f, 2.0f, 1.0f));

	u32 batches{ 0 };
	for (u32 lastIndex = 0; lastIndex < commands.size(); batches++) {
		gl::setBlending(blendingStack.top());
		gl::setDepthTest(depthTestStack.top());
		spriteShader.setUniform(6, &mtsdfSmoothingStack.top());

		lastIndex = drawBatch(commands, worldViewProjMat, pixelViewProjMatrix, lastIndex, context.mainFrameBuffer);
	}

	texLock.unlock();
	bInUse = false;
}

size_t SpritePipe::drawBatch(std::vector<Command>& commands, Mat4 const& worldVPMat, Mat4 const& pixelVPMat, size_t index, gl::Framebuffer& framebuffer)
{
	instances.clear();
	spriteShader.use();

	// give the shader the possible texture slots
	constexpr std::array<int, 32> texSamplers = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	spriteShader.setUniform(50, reinterpret_cast<int const*>(&texSamplers), 32);

	bool errorTexLoaded{ false };

	TextureSamplerManager samplerManager{ *texBackend, texLock, 32 };
	int spriteCount{ 0 };
	for (; index < commands.size() && spriteCount < MAX_BATCH_SIZE; index++) {
		if (Sprite* sprite = std::get_if<Sprite>(&commands[index])) {
			int thisSpriteTexSampler{ -1 };		// -1 := no texture; -2 := could not access texture
			if (sprite->texHandle.holdsValue()) {
				auto sampler = samplerManager.getSampler(sprite->texHandle);
				if (sampler.has_value())
					thisSpriteTexSampler = sampler.value();
				else
					break;	/* there are no samplers left and we need another one, so we can not draw this sprite in this batch */
			}
			if (thisSpriteTexSampler == -2) sprite->color = Vec4(1, 0, 1, 1);
			genInstance(*sprite, thisSpriteTexSampler, worldVPMat, pixelVPMat, instances);
			spriteCount++;
		}
		else if (gl::BlendingFunction* blending = std::get_if<gl::BlendingFunction>(&commands[index])) {
			blendingStack.push(*blending);
			index++;
			break;
		}
		else if (std::holds_alternative<PopBlendingFunction>(commands[index])) {
			blendingStack.pop();
			index++;
			break;
		}
		else if (std::holds_alternative<EndBatchCmd>(commands[index])) {
			index++;
			break;
		}
		else {
			assert(false);
		}
	}


	f32 uniformXBend = cast<f32>(framebuffer.getSize().second) / cast<f32>(framebuffer.getSize().first);
	Mat4 uniformWindowSpaceMatrix = Mat4::scale({ uniformXBend, 1.0f, 1.0f }); 
	auto [width, height] = framebuffer.getSize();
	spriteShader.setUniform(0, &worldVPMat);
	spriteShader.setUniform(2, &uniformWindowSpaceMatrix);
	spriteShader.setUniform(3, &pixelVPMat);
	spriteShader.setUniform(4, &width);    
	spriteShader.setUniform(5, &height);
	spriteShader.setUniform(7, &sdfBarrier);


	spriteShader.bufferInstancedVertices(instances.size(), instances.data());
	spriteShader.drawInstanced(6, spriteCount, framebuffer);

	trisDrawn += 2 * spriteCount;
	return index;
}

void SpritePipe::genInstance(Sprite const& d, int samplerSlot, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, std::vector<SpriteInstance>& vertexBuffer)
{
	vertexBuffer.emplace_back(SpriteInstance{
		.color = d.color,
		.position = { d.position.x, d.position.y, d.position.z, 0 },
		.rotation = d.rotationVec.toUnitX0(),
		.scale = d.scale,
		.texMin = d.texMin,
		.texMax = d.texMax,
		.clipMin = d.clipMin,
		.clipMax = d.clipMax,
		.texSampler = samplerSlot,
		.isMSDF = static_cast<GLint>(d.isMSDF),
		.cornerRounding = d.cornerRounding,
		.renderSpace = static_cast<GLint>(d.drawMode),
	});
}

void SpritePipe::initializeSpriteShader()
{
	std::array<u32, 6> vertexIndices;
	vertexIndices[0] = 0;
	vertexIndices[1] = 1;
	vertexIndices[2] = 2;
	vertexIndices[3] = 2;
	vertexIndices[4] = 1;
	vertexIndices[5] = 3;

	std::array<SpriteVertex, 4> vertecies;
	vertecies[0] = SpriteVertex{ Vec2{ -0.5f, +0.5f }, Vec2{ 0.0f, 1.0f } };
	vertecies[1] = SpriteVertex{ Vec2{ +0.5f, +0.5f }, Vec2{ 1.0f, 1.0f } };
	vertecies[2] = SpriteVertex{ Vec2{ -0.5f, -0.5f }, Vec2{ 0.0f, 0.0f } };
	vertecies[3] = SpriteVertex{ Vec2{ +0.5f, -0.5f }, Vec2{ 1.0f, 0.0f } };

	spriteShader.initialize(readShader(SHADER_SRC_VERTEX_PATH), readShader(SHADER_SRC_FRAGMENT_PATH));
	spriteShader.setVertexAttributes<SPRITE_VERTEX_ATTRIBUTES>();
	spriteShader.setInstanceVertexAttributes<SPRITE_INSTANCE_VERTEX_ATTRIBUTES>();

	spriteShader.bufferIndices(6, reinterpret_cast<u32*>(&vertexIndices));
	spriteShader.bufferVertices(4, reinterpret_cast<void*>(&vertecies));
}

void SpritePipe::push(std::vector<Command>& commandsIn)
{
	std::swap(commands, commandsIn);
	commandsIn.clear();
}

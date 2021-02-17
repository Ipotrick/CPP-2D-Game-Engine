#include "SpriteRenderPipe.hpp"

#include "../TextureSamplerManager.hpp"

void SpriteRenderPipe::Backend::init(RenderPipeContext& context)
{
	bInUse = true;

	// querry driver and hardware info:
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);

	initializeSpriteShader();
	bInUse = false;
}

void SpriteRenderPipe::Backend::reset(RenderPipeContext& context)
{
	bInUse = true;
	texLock.lock();
	texBackend->unloadQueue(texLock);
	texLock.unlock();
	bInUse = false;
}

void SpriteRenderPipe::Backend::render(RenderPipeContext& context)
{
	bInUse = true;
	texLock.lock();
	texBackend->unloadQueue(texLock);
	texBackend->loadQueue(texLock);

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

	if (bClearDepthBeforeDraw) context.mainFrameBuffer.clearDepth();

	u32 batches{ 0 };
	for (u32 lastIndex = 0; lastIndex < spritesBack.size(); batches++) {
		lastIndex = drawBatch(spritesBack, worldViewProjMat, pixelViewProjMatrix, lastIndex, context.mainFrameBuffer);
	}

	texLock.unlock();
	bInUse = false;
}

size_t SpriteRenderPipe::Backend::drawBatch(std::vector<Sprite>& sprites, Mat4 const& worldVPMat, Mat4 const& pixelVPMat, size_t index, OpenGLFrameBuffer& framebuffer)
{
	spriteShaderCPUModelBuffer.clear();
	spriteShaderCPUVertexBuffer.clear();
	spriteShader.use();

	// give the shader the possible texture slots
	constexpr std::array<int, 32> texSamplers = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	spriteShader.setUniform(50, reinterpret_cast<int const*>(&texSamplers), 32);

	bool errorTexLoaded{ false };

	TextureSamplerManager samplerManager{ *texBackend, texLock, 32 };
	int spriteCount{ 0 };
	for (; index < sprites.size() && spriteCount < MAX_RECT_COUNT; index++, spriteCount++) {
		int thisSpriteTexSampler{ -1 };		// -1 := no texture; -2 := could not access texture
		if (sprites[index].texHandle.holdsValue()) {
			auto sampler = samplerManager.getSampler(sprites[index].texHandle);
			if (sampler.has_value())
				thisSpriteTexSampler = sampler.value();
			else
				break;	/* there are no samplers left and we need another one, so we can not draw this sprite in this batch */
		}
		if (thisSpriteTexSampler == -2) sprites[index].color = Vec4(1, 0, 1, 1);
		generateVertices(sprites[index], thisSpriteTexSampler, worldVPMat, pixelVPMat, spriteShaderCPUVertexBuffer);
	}
	spriteShader.setUniform(0, &worldVPMat);
	f32 uniformXBend = cast<f32>(framebuffer.getSize().second) / cast<f32>(framebuffer.getSize().first);
	Mat4 uniformWindowSpaceMatrix = Mat4::scale({ uniformXBend, 1.0f, 1.0f });
	spriteShader.setUniform(2, &uniformWindowSpaceMatrix);
	spriteShader.setUniform(3, &pixelVPMat);
	auto [width, height] = framebuffer.getSize();
	spriteShader.setUniform(4, &width);
	spriteShader.setUniform(5, &height);
	const f32 MTSDF_SMOOTHING = 1.75f;
	spriteShader.setUniform(6, &MTSDF_SMOOTHING);
	spriteShader.bufferVertices(spriteShaderCPUVertexBuffer.size(), spriteShaderCPUVertexBuffer.data());
	modelSSBO.buffer(spriteCount, spriteShaderCPUModelBuffer.data());
	spriteShader.renderTo((size_t)spriteCount * (size_t)6, spriteShaderIndices.data(), framebuffer);
	return index;
}

void SpriteRenderPipe::Backend::generateVertices(Sprite const& d, int samplerSlot, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, std::vector<SpriteShaderVertex>& vertexBuffer)
{
	spriteShaderCPUModelBuffer.push_back({});
	auto& model = spriteShaderCPUModelBuffer.back();
	model.color = d.color;
	model.position = { d.position.x, d.position.y, d.position.z, 0 };
	model.rotation = d.rotationVec.toUnitX0();
	model.scale = d.scale;
	model.texMin = d.texMin;
	model.texMax = d.texMax;
	model.texSampler = samplerSlot;
	model.isMSDF = static_cast<GLint>(d.isMSDF);
	model.cornerRounding = d.cornerRounding;
	model.renderSpace = static_cast<GLint>(d.drawMode);

	int32_t modelIndex = (uint32_t)spriteShaderCPUModelBuffer.size() - 1;

	for (int i = 0; i < 4; i++) {
		vertexBuffer.emplace_back(SpriteShaderVertex{ i, modelIndex });
	}
}

void SpriteRenderPipe::Backend::initializeSpriteShader()
{
	for (int sprite = 0; sprite < MAX_RECT_COUNT; sprite++) {
		for (int triangle = 0; triangle < 2; triangle++) {
			for (int j = 0 + triangle; j < (3 + triangle); j++) {
				spriteShaderIndices.push_back(j + sprite * 4);
			}
		}
	}

	spriteShader.initialize(readShader(SPRITE_SHADER_VERTEX_PATH), readShader(SPRITE_SHADER_FRAGMENT_PATH));
	spriteShader.setVertexAttributes< s32, s32 >(MAX_VERTEX_COUNT);
}

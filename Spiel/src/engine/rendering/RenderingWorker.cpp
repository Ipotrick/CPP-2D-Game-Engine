#include "RenderingWorker.hpp"

#include <sstream>
#include <fstream>

#include "../types/Timing.hpp"

#include "OpenGLShader.hpp"


void openGLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* msg, const void* data)
{
	const char* _source;
	const char* _type;
	const char* _severity;

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		_source = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN"; break;
	default:
		_source = "UNKNOWN"; break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR"; break;
	case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER"; break;
	case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER"; break;
	default:
		_type = "UNKNOWN"; break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM"; break;
	case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION"; break;
	default:
		_severity = "UNKNOWN"; break;
	}

	if (std::strcmp(_severity,"NOTIFICATION")) {
		printf("OpenGL error [%d]: %s of %s severity, raised from %s: %s\n",
			id, _type, _severity, _source, msg);
		__debugbreak();
	}
}

void RenderingWorker::initialize(Window* window)
{
	assert(!bInitialized);
	bInitialized = true;
	window->takeRenderingContext();

	// opengl configuration:
	glEnable(GL_BLEND); 
	glBlendFunc(BLEND_SFACTOR, BLEND_DFACTOR);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(openGLDebugMessageCallback, 0);
#endif

	// querry driver and hardware info:
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);

	initializeSpriteShader();

	mainFramebuffer.initialize();
	passShader.initialize(PASS_SHADER_FRAGMENT_PATH);
	texCache.initialize();

	std::unique_lock switch_lock(data->mut);
	data->run = true;
	data->state = SharedRenderData::State::waitForFrontEnd;

}

void RenderingWorker::update(Window* window)
{
	/**
	 * TODO
	 * The reset function is bugged.
	 * when resetting things like texture cache are not reset correctly:
	 * a texcache reset should invalidate all textures (all opengl ressources), so that they are loaded newly.
	 */

	//if (windowOfLastFrame != window) /* when the window changes, we need to reset and initialize new with the rendering context of the new window */
	//{
	//	if (windowOfLastFrame != nullptr) {
	//		reset(windowOfLastFrame);
	//	}
	//	initialize(window);
	//}

	// update framebuffer sizes to window size:
	auto [width, height] = window->getSize();
	if (width != lastWindowWidth or height != lastWindowHeight) {
		lastWindowWidth = width;
		lastWindowHeight = height;
		bWindowSizeChanged = true;
		uint32_t ssWidth = supersamplingFactor * lastWindowWidth;
		uint32_t ssHeight = supersamplingFactor * lastWindowHeight;
		mainFramebuffer.resize(ssWidth, ssHeight);
	}
	else {
		bWindowSizeChanged = false;
	}

	// Render current frame:
	Timer t(data->new_renderTime);

	if (data->renderBuffer->resetTextureCache) {
		texCache.reset();
	}

	texCache.cacheTextures(data->renderBuffer->textureLoadingQueue);

	deadScriptsOnDestroy();
	newScriptsOnInitialize();

	data->drawCallCount = 0;
	data->spriteCount = 0;
	mainFramebuffer.clear();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& camera = data->renderBuffer->camera;
	Mat4 worldViewProjMat = Mat4::scale(camera.zoom) * Mat4::scale({ camera.frustumBend.x, camera.frustumBend.y, 1.0f }) * Mat4::rotate_z(-camera.rotation) * Mat4::translate({ -camera.position.x, -camera.position.y, 0.0f });
	Mat4 pixelViewProjMatrix = Mat4::translate(Vec3(-1, -1, 0)) * Mat4::scale(Vec3(1.0f / (lastWindowWidth), 1.0f / (lastWindowHeight), 1.0f)) * Mat4::scale(Vec3(2.0f, 2.0f, 1.0f));

	for (auto& layer : data->renderBuffer->layers) {
		drawLayer(layer, worldViewProjMat, pixelViewProjMatrix);
	}

	passShader.renderTexToFBO(mainFramebuffer.getTex(), 0, lastWindowWidth, lastWindowHeight);

	window->swapBuffers();
}

void RenderingWorker::operator()()
{
	initialize(data->window);
	
	while (waitForFrontend()) {
		update(data->window);
	}

	reset(data->window);
}

void RenderingWorker::reset(Window* window)
{
	assert(bInitialized);
	bInitialized = false;
	mainFramebuffer.reset();
	spriteShaderModelSSBO.reset();
	passShader.reset();
	windowOfLastFrame = nullptr;
	texCache.reset();

	std::unique_lock<std::mutex> switch_lock(data->mut);
	data->run = false;
	data->state = SharedRenderData::State::reset;
	data->cond.notify_one();
	window->releaseRenderingContext();
}

void RenderingWorker::drawLayer(RenderLayer& layer, Mat4 const& cameraViewProj, Mat4 const& pixelProjectionMatrix)
{
	if (layer.bSortForDepth) {
		std::stable_sort(layer.getSprites().begin(), layer.getSprites().end());
	}

	// set layer depth test:
	glDepthFunc(static_cast<GLuint>(layer.depthTest));

	// batch render sprites:
	for (int lastIndex = 0; lastIndex < layer.getSprites().size(); data->drawCallCount += 1) {
		lastIndex = drawBatch(layer.getSprites(), cameraViewProj, pixelProjectionMatrix, lastIndex, mainFramebuffer);
	}

	if (layer.script) {
		layer.script->onUpdate(*this, layer);
	}
}

bool RenderingWorker::waitForFrontend()
{
	Timer t(data->new_renderSyncTime);
	std::unique_lock switch_lock(data->mut);
	data->state = SharedRenderData::State::waitForFrontEnd;
	data->cond.notify_one();
	data->cond.wait(switch_lock, [&]() { return data->state == SharedRenderData::State::running; });
	return data->run;
}

size_t RenderingWorker::drawBatch(std::vector<Sprite>& sprites, Mat4 const& worldVPMat, Mat4 const& pixelVPMat, size_t index, OpenGLFrameBuffer& framebuffer)
{
	spriteShaderCPUModelBuffer.clear();
	spriteShaderCPUVertexBuffer.clear();
	spriteShader.use();

	// give the shader the possible texture slots
	constexpr std::array<int,32> texSamplers = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	spriteShader.setUniform(50, reinterpret_cast<int const*>(&texSamplers), 32);

	texCache.getTexture(TextureCache::DEFAULT_TEXTURE_ID).bindToSampler(0);

	TextureSamplerManager samplerManager{ texCache, 32 };
	int spriteCount{ 0 };
	for (; (index < sprites.size()) & (spriteCount < MAX_RECT_COUNT); index++, spriteCount++) {
		int thisSpriteTexSampler{ -1 };
		if (sprites[index].texRef.has_value()) {
			auto sampler = samplerManager.getSampler(sprites[index].texRef.value());
			if (sampler.has_value())
				thisSpriteTexSampler = sampler.value();
			else
				break;	/* there are no samplers left and we need another one, so we can not draw this sprite in this batch */
		}
		generateVertices(sprites[index], thisSpriteTexSampler, worldVPMat, pixelVPMat, spriteShaderCPUVertexBuffer);
	}
	spriteShader.setUniform(0, &worldVPMat);
	Mat4 uniformWindowSpaceMatrix = Mat4::scale({ data->renderBuffer->camera.frustumBend.x,data->renderBuffer->camera.frustumBend.y, 1.0f });
	spriteShader.setUniform(2, &uniformWindowSpaceMatrix);
	spriteShader.setUniform(3, &pixelVPMat);
	spriteShader.bufferVertices(spriteShaderCPUVertexBuffer.size(), spriteShaderCPUVertexBuffer.data());
	spriteShaderModelSSBO.buffer(spriteCount, spriteShaderCPUModelBuffer.data());
	spriteShader.renderTo((size_t)spriteCount * (size_t)6, spriteShaderIndices.data(), framebuffer);
	data->spriteCount += spriteCount;
	return index;
}

void RenderingWorker::generateVertices(Sprite const& d, int samplerSlot, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, std::vector<SpriteShaderVertex>& vertexBuffer) {
	Vec2 minTex{ 0,0 };
	Vec2 maxTex{ 1,1 };
	if (d.texRef.has_value()) {
		minTex = d.texRef.value().minPos;
		maxTex = d.texRef.value().maxPos;
	}

	spriteShaderCPUModelBuffer.push_back({});
	auto& model = spriteShaderCPUModelBuffer.back();
	model.color = d.color;
	model.position = { d.position.x, d.position.y, -d.position.z, 0};
	model.rotation = d.rotationVec.toUnitX0();
	model.scale = d.scale;
	model.texMin = minTex;
	model.texMax = maxTex;
	model.texSampler = samplerSlot;
	model.isCircle = static_cast<GLint>(d.form);
	model.renderSpace = static_cast<GLint>(d.drawMode);

	int32_t modelIndex = (uint32_t)spriteShaderCPUModelBuffer.size() - 1;

	for (int i = 0; i < 4; i++) {
		vertexBuffer.emplace_back(SpriteShaderVertex{ i, modelIndex });
	}
}

void RenderingWorker::newScriptsOnInitialize()
{
	for (auto& layer : data->renderBuffer->layers) {
		if (layer.script && !layer.script->isInitialized()) {
			layer.script->onInitialize(*this, layer);
			layer.script->bInitialized = true;
		}
	}
}

void RenderingWorker::deadScriptsOnDestroy()
{
	for (auto& script : data->renderBuffer->scriptDestructQueue) {
		script->onDestroy(*this);
	}
	data->renderBuffer->scriptDestructQueue.clear();
}

void RenderingWorker::initializeSpriteShader()
{
	for (int sprite = 0; sprite < MAX_RECT_COUNT; sprite++) {
		for (int triangle = 0; triangle < 2; triangle++) {
			for (int j = 0 + triangle; j < (3 + triangle); j++) {
				spriteShaderIndices.push_back(j + sprite * 4);
			}
		}
	}

	spriteShader.initialize(readShader(SPRITE_SHADER_VERTEX_PATH), readShader(SPRITE_SHADER_FRAGMENT_PATH));
	spriteShader.setVertexAttributes< int, int >(MAX_VERTEX_COUNT);
	spriteShaderModelSSBO.initialize(MAX_RECT_COUNT, spriteShader, SPRITE_SHADER_MODEL_SSBO_BINDING);
}
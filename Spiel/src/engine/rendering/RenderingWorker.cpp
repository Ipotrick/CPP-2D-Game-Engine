#include "RenderingWorker.hpp"

#include <sstream>
#include <fstream>

#include "../types/Timing.hpp"

void RenderingWorker::initiate()
{
	std::lock_guard<std::mutex> l(window->renderingContextMut); 
	window->makeContextCurrent();

	// opengl configuration:
	glEnable(GL_BLEND); 
	glBlendFunc(BLEND_SFACTOR, BLEND_DFACTOR);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// querry driver and hardware info:
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);

	//initializeFBOs();
	mainTFBO.initialize();
	initializeSpriteShader(); 
	passShader.initialize(PASS_SHADER_FRAGMENT_PATH);
	texCache.initialize();

	std::unique_lock switch_lock(data->mut);
	data->run = true;
	data->state = SharedRenderData::State::waitForFrontEnd;
}

void RenderingWorker::operator()()
{
	initiate();
	
	while (waitForFrontend()) {
		{	// process render side events
			std::lock_guard l(window->mut);
			window->updateSize();
			auto [width, height] = window->getSize();

			if (width != lastWindowWidth or height != lastWindowHeight) {
				lastWindowWidth = width;
				lastWindowHeight = height;
				bWindowSizeChanged = true;
				uint32_t ssWidth = supersamplingFactor * width;
				uint32_t ssHeight = supersamplingFactor * height;

				// resize main fbo texture:
				mainTFBO.resize(ssWidth, ssHeight);
			}
			else {
				bWindowSizeChanged = false;
			}
		}
		{	// render frame
			std::lock_guard renderContextLock(window->renderingContextMut);		// when we have two renderers we have to make sure that only one is calling opengl functions on the window's context
			Timer t(data->new_renderTime);
			// takeRenderingContext();											// this is only needed when multiple renderers are rendering to a window

			if (data->renderBuffer->resetTextureCache) {
				texCache.reset();
			}

			texCache.cacheTextures(data->renderBuffer->textureLoadingQueue);

			deadScriptsOnDestroy();

			newScriptsOnInitialize();

			data->drawCallCount = 0;

			auto& camera = data->renderBuffer->camera;
			Mat4 worldViewProjMat = 
				Mat4::scale(camera.zoom) *												
				Mat4::scale({ camera.frustumBend.x, camera.frustumBend.y, 1.0f }) *
				Mat4::rotate_z(-camera.rotation) *
				Mat4::translate({ -camera.position.x, -camera.position.y, 0.0f });
			Mat4 pixelViewProjMatrix = 
				Mat4::translate(Vec3(-1, -1, 0)) * 
				Mat4::scale(Vec3(1.0f / (window->getWidth()), 1.0f / (window->getHeight()), 1.0f)) *
				Mat4::scale(Vec3(2.0f, 2.0f, 1.0f));

			clearMainFBO();
			mainTFBO.clear();

			for (auto& layer : data->renderBuffer->layers) {
				drawLayer(layer, worldViewProjMat, pixelViewProjMatrix);
			}

			passShader.renderTexToFBO(mainTFBO.getTex(), 0, window->getWidth(), window->getHeight());

			window->swapBuffers();
		}
	}

	reset();
}

void RenderingWorker::reset() {
	passShader.reset();

	free(spriteShaderVBORaw);
	free(indicesRaw);
	free(modelSSBORaw);

	mainTFBO.reset();

	std::unique_lock<std::mutex> switch_lock(data->mut);
	data->run = false;
	data->state = SharedRenderData::State::reset;
	data->cond.notify_one();
}

void RenderingWorker::drawLayer(RenderLayer& layer, Mat4 const& cameraViewProj, Mat4 const& pixelProjectionMatrix)
{
	if (layer.bSortForDepth) {
		std::stable_sort(layer.getSprites().begin(), layer.getSprites().end());
	}

	// set current render target to the main texture frame buffer object
	glBindFramebuffer(GL_FRAMEBUFFER, mainTFBO.getBuffer());

	// set layer depth test:
	glDepthFunc(static_cast<GLuint>(layer.depthTest));

	// batch render sprites:
	for (int lastIndex = 0; lastIndex < layer.getSprites().size(); data->drawCallCount += 1) {
		lastIndex = drawBatch(layer.getSprites(), cameraViewProj, pixelProjectionMatrix, lastIndex);
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

size_t RenderingWorker::drawBatch(std::vector<Sprite>& drawables, Mat4 const& worldVPMat, Mat4 const& pixelVPMat, size_t startIndex)
{
	nextModelIndex = 0;
	glUseProgram(spriteShaderProgram);
	glBindVertexArray(spriteShaderVAO);

	// give the shader the possible texture slots
	int texSamplers[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	glUniform1iv(50, 32, texSamplers);

	bindTexture(TEXTURE_DEFAULT, 0);

	uint32_t nextTextureSamplerSlot{ 1 }; // when there are no texture slots left, the batch is full and will be rendered.
	robin_hood::unordered_map<TextureId, GLuint> usedTexturesSamplerSlots;	// when a texture is used it will get a slot 
	// fill batch with vertices
	int spriteCount{ 0 };
	size_t index{ startIndex };
	for (; (index < drawables.size()) & (spriteCount < MAX_RECT_COUNT); index++, spriteCount++) {
		Sprite const& thisSprite = drawables[index];
		int thisSpriteTexSampler{ 0 };

		// check if drawable has texture
		if (thisSprite.texRef.has_value()) {
			auto& texRef = thisSprite.texRef.value();
			if (texCache.isTextureLoaded(texRef) && texCache.getTexture(texRef).good) {
				// is texture allready in the usedSamplerMap?
				if (usedTexturesSamplerSlots.contains(texRef.id)) {
					// then just use the allready sloted texture sampler:
					thisSpriteTexSampler = usedTexturesSamplerSlots[texRef.id];
				}
				else {
					// if all texture sampler slots are used flush the batch
					if (nextTextureSamplerSlot >= maxTextureSlots) break;

					// there is a free texture sampler slot
					// add texture to usedSamplerSlotMap:
					usedTexturesSamplerSlots.insert({ texRef.id , nextTextureSamplerSlot });
					// bind texture to sampler slot
					bindTexture(texCache.getTexture(texRef).openglTexID, nextTextureSamplerSlot);
					thisSpriteTexSampler = nextTextureSamplerSlot;
					nextTextureSamplerSlot++;
				}
			}
			else {
				// drawable gets the error texture
				thisSpriteTexSampler = 0;
			}
		}
		else {
			// drawable has no texture so it gets sampler slot -1 (white texture)
			thisSpriteTexSampler = -1;
		}

		auto bufferPtr = (SpriteShaderVertex*)(spriteShaderVBORaw + spriteCount * SpriteShaderVertex::FLOAT_SIZE * 4);	// gets index for the next 4 vertecies in raw buffer
		generateVertices(thisSprite, thisSpriteTexSampler, worldVPMat, pixelVPMat, bufferPtr);
	}
	// set projection-matrix-uniforms:
	glUniformMatrix4fv(0, 1, GL_FALSE, worldVPMat.data());
	// the window space matrix has no uniform as it is the identity matrix
	Mat4 uniformWindowSpaceMatrix = Mat4::scale({ data->renderBuffer->camera.frustumBend.x,data->renderBuffer->camera.frustumBend.y, 1.0f });
	glUniformMatrix4fv(2, 1, GL_FALSE, uniformWindowSpaceMatrix.data());
	glUniformMatrix4fv(3, 1, GL_FALSE, pixelVPMat.data());

	// push vertex data to the gpu
	glBindBuffer(GL_ARRAY_BUFFER, spriteShaderVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(SpriteShaderVertex) * spriteCount * 4, spriteShaderVBORaw);

	// push model data to gpu
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SpriteShaderModel) * spriteCount, modelSSBORaw);

	// render drawableCount amount of drawables with glDrawElements
	glViewport(0, 0, window->getWidth() * supersamplingFactor, window->getHeight() * supersamplingFactor);
	glDrawElements(GL_TRIANGLES, spriteCount * 6, GL_UNSIGNED_INT, indicesRaw);
	return index;
}

void RenderingWorker::generateVertices(Sprite const& d, float texID, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, SpriteShaderVertex* bufferPtr) {
	Vec2 minTex{ 0,0 };
	Vec2 maxTex{ 1,1 };
	if (d.texRef.has_value()) {
		minTex = d.texRef.value().minPos;
		maxTex = d.texRef.value().maxPos;
	}

	bool isCircle = static_cast<bool>(d.form);

	SpriteShaderModel* model = modelSSBORaw + nextModelIndex;
	model->color = d.color;
	model->position = { d.position.x, d.position.y, -d.position.z, 0};
	model->rotation = d.rotationVec.toVec2();
	model->scale = d.scale;
	model->texId = texID;
	model->isCircle = isCircle;
	model->renderSpace = static_cast<GLint>(d.drawMode);

	SpriteShaderVertex* vertex{ nullptr };
#define GEN_VERTEX(index) \
	vertex = bufferPtr + index;\
	vertex->texCoord = idToCorner(index, minTex, maxTex);\
	vertex->corner = idToCorner(index, { -0.5f, -0.5f }, { 0.5f, 0.5f });\
	vertex->modelIndex = nextModelIndex;
	
	GEN_VERTEX(0);
	GEN_VERTEX(1);
	GEN_VERTEX(2);
	GEN_VERTEX(3);

	++nextModelIndex;
}

void RenderingWorker::bindTexture(GLuint texID, int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texID);
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
	auto spriteShaderVertex = readShader(SPRITE_SHADER_VERTEX_PATH);
	auto spriteShaderFragment = readShader(SPRITE_SHADER_FRAGMENT_PATH);
	spriteShaderProgram = createOGLShaderProgram(spriteShaderVertex, spriteShaderFragment);

	indicesRaw = (uint32_t*)malloc(sizeof(uint32_t) * MAX_INDEX_COUNT);
	int quadCount = 0;
	for (int i = 0; i < MAX_INDEX_COUNT; i += 6) {
		indicesRaw[i + 0] = 0 + quadCount * 4; indicesRaw[i + 1] = 1 + quadCount * 4; indicesRaw[i + 2] = 2 + quadCount * 4;
		indicesRaw[i + 3] = 1 + quadCount * 4; indicesRaw[i + 4] = 2 + quadCount * 4; indicesRaw[i + 5] = 3 + quadCount * 4;
		quadCount++;
	}

	spriteShaderVBORaw = (float*)malloc(sizeof(SpriteShaderVertex) * MAX_VERTEX_COUNT);
	glGenBuffers(1, &spriteShaderVBO);
	glBindBuffer(GL_ARRAY_BUFFER, spriteShaderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteShaderVertex) * MAX_VERTEX_COUNT, nullptr, GL_DYNAMIC_DRAW);

	// configure vertex attributes
	glGenVertexArrays(1, &spriteShaderVAO);
	glBindVertexArray(spriteShaderVAO);
	// corner coordinates (-0.5, 0,5), (0.5, 0.5), ...
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteShaderVertex), 0);
	// texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteShaderVertex), (const void*)offsetof(SpriteShaderVertex, texCoord));
	// model index / id
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(SpriteShaderVertex), (const void*)offsetof(SpriteShaderVertex, modelIndex));

	// initialize model shader storage object
	modelSSBORaw = (SpriteShaderModel*)malloc(sizeof(SpriteShaderModel) * MAX_RECT_COUNT);
	glGenBuffers(1, &modelSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SpriteShaderModel) * MAX_RECT_COUNT, modelSSBORaw, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MODEL_SSBO_BINDING, modelSSBO);
}

void RenderingWorker::clearMainFBO()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
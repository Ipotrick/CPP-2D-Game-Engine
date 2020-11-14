#include "RenderingWorker.hpp"

#include <sstream>
#include <fstream>

#include "Timing.hpp"


Vec2 idToCorner(int id, Vec2 min, Vec2 max)
{
	switch (id) {
	case 0:
		return { min.x, max.y };	// tl
	case 1:
		return { max.x, max.y };	// tr
	case 2:
		return { min.x, min.y };	// bl
	case 3:
		return { max.x, min.y };	// br
	default:
		assert(false);
		return { 0,0 };
	}
}


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

static unsigned int compileShader(unsigned int type_, const std::string source_) {
	unsigned id = glCreateShader(type_);
	char const* src = source_.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)_malloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cerr << "error: failed to compile shader: " << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned createShader(const std::string& vertexShader_, const std::string& fragmentShader_) {
	unsigned program = glCreateProgram();
	unsigned vs = compileShader(GL_VERTEX_SHADER, vertexShader_);
	unsigned fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader_);

	assert(vs != 0);
	assert(fs != 0);

	glAttachShader(program, vs);
	glCheckError();
	glAttachShader(program, fs);
	glCheckError();
	glLinkProgram(program);
	glCheckError();
	glValidateProgram(program);
	glCheckError();

	glDeleteShader(vs);
	glCheckError();
	glDeleteShader(fs);
	glCheckError();
	return program;
}

void RenderingWorker::initiate()
{
	{
		std::lock_guard<std::mutex> l(window->mut);
		glfwMakeContextCurrent(window->glfwWindow);

		if (glewInit() != GLEW_OK) {
			glfwTerminate();
		}
	}

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);

	auto spriteShaderVertex = readShader(SPRITE_SHADER_VERTEX_PATH);
	auto spriteShaderFragment = readShader(SPRITE_SHADER_FRAGMENT_PATH);
	spriteShaderProgram = createShader(spriteShaderVertex, spriteShaderFragment);
	// enable blending when drawing a fragment twice
	glEnable(GL_BLEND);
	// change blending type to transparency blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_MULTISAMPLE);

	texCache.initialize();

	indicesRaw = (uint32_t*)malloc(sizeof(uint32_t) * MAX_INDEX_COUNT);
	int quadCount = 0;
	for (int i = 0; i < MAX_INDEX_COUNT; i += 6) {
		indicesRaw[i + 0] = 0 + quadCount * 4; indicesRaw[i + 1] = 1 + quadCount * 4; indicesRaw[i + 2] = 2 + quadCount * 4;
		indicesRaw[i + 3] = 1 + quadCount * 4; indicesRaw[i + 4] = 2 + quadCount * 4; indicesRaw[i + 5] = 3 + quadCount * 4;
		quadCount++;
	}

	spriteShaderVBORaw = (float*)malloc(sizeof(Vertex) * MAX_VERTEX_COUNT);
	glGenBuffers(1, &spriteShaderVBO);
	glBindBuffer(GL_ARRAY_BUFFER, spriteShaderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERTEX_COUNT, nullptr, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &spriteShaderVAO);
	glBindVertexArray(spriteShaderVAO);

	// corner coordinates (-0.5, 0,5), (0.5, 0.5), ...
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	// texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoord));
	// model index / id
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (const void*)offsetof(Vertex, modelIndex));



	//// positions (2 float)
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	//// vertex color (4 float)
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));
	//// texture uv coordinates (2 float)
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoord));
	//// corner coordiantes
	//glEnableVertexAttribArray(5);
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, corner));
	//// texture slot (1 int) 
	//glEnableVertexAttribArray(3);
	//glVertexAttribIPointer(4, 1, GL_INT, sizeof(Vertex), (const void*)offsetof(Vertex, texID));
	//// circle rendering mode enable
	//glEnableVertexAttribArray(4);
	//glVertexAttribIPointer(5, 1, GL_INT, sizeof(Vertex), (const void*)offsetof(Vertex, circle));
	//// model ssbo index
	//glEnableVertexAttribArray(6);
	//glVertexAttribIPointer(6, 1, GL_INT, sizeof(Vertex), (const void*)offsetof(Vertex, modelID));

	modelSSBORaw = (RenderModel*)malloc(sizeof(RenderModel) * MAX_RECT_COUNT);
	glGenBuffers(1, &modelSSBO);
	glCheckError();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSSBO);
	glCheckError();
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(RenderModel) * MAX_RECT_COUNT, modelSSBORaw, GL_DYNAMIC_COPY);
	glCheckError();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glCheckError();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MODEL_SSBO_BINDING, modelSSBO);
	glCheckError();
}

void RenderingWorker::operator()()
{
	initiate();
	
	while (data->run) {
		auto& camera = data->renderBuffer->camera; 
		{	
			Timer t(data->new_renderTime);

			if (data->renderBuffer->resetTextureCache) {
				texCache.reset();
			}

			// refresh texture Refs

			texCache.cacheTextures(data->renderBuffer->textureLoadingQueue);

			Mat4 viewProjectionMatrix = 
				Mat4::scale(camera.zoom) *												
				Mat4::scale({ camera.frustumBend.x, camera.frustumBend.y, 1.0f }) *
				Mat4::rotate_z(-camera.rotation) *
				Mat4::translate({ -camera.position.x, -camera.position.y, 0.0f });
			Mat4 pixelProjectionMatrix = 
				Mat4::translate(Vec3(-1, -1, 0)) * 
				Mat4::scale(Vec3(1.0f / window->width, 1.0f / window->height, 1.0f)) * 
				Mat4::scale(Vec3(2.0f, 2.0f, 1.0f));

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			data->drawCallCount = 0;

			for (auto& layer : data->renderBuffer->layers) {
				drawLayer(layer, viewProjectionMatrix, pixelProjectionMatrix);
			}

			{	// push rendered image into image buffer
				std::lock_guard<std::mutex> l(window->mut);
				glfwSwapBuffers(window->glfwWindow);
			}
		}

		{	// process render side events
			std::lock_guard<std::mutex> l(window->mut);
			int width, height;
			glfwGetWindowSize(window->glfwWindow, &width, &height);
			glViewport(0, 0, window->width, window->height);
			window->width = width;
			window->height = height;
		}

		{	// wait for main
			Timer t(data->new_renderSyncTime);
			std::unique_lock<std::mutex> switch_lock(data->mut);
			data->ready = true;
			data->cond.notify_one();
			data->cond.wait(switch_lock, [&]() { return data->ready == false; });
		}
	}

	end();
}

void RenderingWorker::end() {
	std::unique_lock<std::mutex> switch_lock(data->mut);
	data->ready = true;
	data->run = false;
	data->cond.notify_one();
}

std::string RenderingWorker::readShader(std::string path_)
{
	std::stringstream ss;
	std::ifstream ifs(path_);
	if (!ifs.good()) {
		std::cerr << "error: could not read shader from path: " << path_ << std::endl;
		data->run = false;
	}
	std::string line;
	while (getline(ifs, line)) {
		ss << line << '\n';
	}
	return ss.str();
}

void RenderingWorker::generateVertices(Drawable const& d, float texID, Mat4 const& viewProjMat, Mat4 const& pixelProjectionMatrix, Vertex* bufferPtr) {
	Vec2 minTex{ 0,0 };
	Vec2 maxTex{ 1,1 };
	if (d.texRef.has_value() && texCache.isTextureLoaded(d.texRef.value())) {
		minTex = d.texRef.value().minPos;
		maxTex = d.texRef.value().maxPos;
	}

	bool isCircle = d.form == Form::Circle ? 1.0f : 0.0f;

	Mat4 vP = Mat4::identity();
	switch (d.getDrawMode()) {
	case RenderSpace::WorldSpace:
		vP = viewProjMat;
		break;
	case RenderSpace::WindowSpace:
		vP = Mat4::identity();
		break;
	case RenderSpace::UniformWindowSpace:
		vP = Mat4::scale({ data->renderBuffer->camera.frustumBend.x,data->renderBuffer->camera.frustumBend.y, 1.0f });
		break;
	case RenderSpace::PixelSpace: 
		vP = pixelProjectionMatrix;
		break;
	default:
		assert(false);
		break;
	}

	RenderModel* model = modelSSBORaw + nextModelIndex;
	model->color = d.color;
	model->position = d.position;
	model->rotation = d.rotationVec.toVec2();
	model->scale = d.scale;
	model->texId = texID;
	model->isCircle = isCircle;
	model->viewProj = vP;

	for (int i = 0; i < 4; ++i) {
		Vertex* vertex = bufferPtr + i;
		vertex->texCoord = idToCorner(i, minTex, maxTex);
		vertex->corner = idToCorner(i, { -0.5f, -0.5f }, { 0.5f, 0.5f });
		vertex->modelIndex = nextModelIndex;
	}
	++nextModelIndex;
}

size_t RenderingWorker::drawBatch(std::vector<Drawable>& drawables, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix, size_t startIndex)
{
	nextModelIndex = 0;
	glUseProgram(spriteShaderProgram);
	glBindVertexArray(spriteShaderVAO);

	// give the shader the possible texture slots
	int texSamplers[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	glUniform1iv(50, 32, texSamplers);

	bindTexture(TEXTURE_DEFAULT, 0);

	uint32_t nextTextureSamplerSlot{ 1 }; // when there are no texture slots left, the batch is full and will be rendered.
	robin_hood::unordered_map<TextureId, uint32_t> usedTexturesSamplerSlots;	// when a texture is used it will get a slot 
	// fill batch with vertices
	size_t index{ startIndex };
	int drawableCount{ 0 };
	for (; index < drawables.size()  && drawableCount < MAX_RECT_COUNT; index++, drawableCount++)
	{
		Drawable const& d = drawables[index];
		int drawableSamplerSlot{ 0 };

		// check if drawable has texture
		if (d.texRef.has_value()) {
			auto& texRef = d.texRef.value();
			if (texCache.isTextureLoaded(texRef) && texCache.getTexture(texRef).good) {
				// is texture allready in the usedSamplerMap?
				if (usedTexturesSamplerSlots.contains(texRef.id)) {
					// then just use the allready sloted texture sampler:
					drawableSamplerSlot = usedTexturesSamplerSlots[texRef.id];
				}
				else { 
					// if all texture sampler slots are used flush the batch
					if (nextTextureSamplerSlot >= maxTextureSlots) break;

					// there is a free texture sampler slot
					// add texture to usedSamplerSlotMap:
					usedTexturesSamplerSlots.insert({ texRef.id , nextTextureSamplerSlot });
					// bind texture to sampler slot
					bindTexture(texCache.getTexture(texRef).openglTexID, nextTextureSamplerSlot);
					drawableSamplerSlot = nextTextureSamplerSlot;
					nextTextureSamplerSlot++;
				}
			}
			else { 
				// drawable gets the error texture
				drawableSamplerSlot = 0;
			}
		}
		else { 
			// drawable has no texture so it gets sampler slot -1 (white texture)
			drawableSamplerSlot = -1;
		}

		auto bufferPtr = (Vertex*)(spriteShaderVBORaw + drawableCount * Vertex::FLOAT_SIZE * 4);	// gets index for the next 4 vertecies in raw buffer
		generateVertices(d, drawableSamplerSlot, viewProjectionMatrix, pixelProjectionMatrix, bufferPtr);
	}
	// push vertex data to the gpu
	glBindBuffer(GL_ARRAY_BUFFER, spriteShaderVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * MAX_VERTEX_COUNT, spriteShaderVBORaw);

	// push model data to gpu
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(RenderModel) * drawableCount, modelSSBORaw);

	// render drawableCount amount of drawables with glDrawElements
	glDrawElements(GL_TRIANGLES, drawableCount * 6, GL_UNSIGNED_INT, indicesRaw);
	return index;
}

// DEPRECATED
void RenderingWorker::bindTexture(GLuint texID, int slot)
{
#ifdef _DEBUG
	int maxTexCount;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexCount);
	assert(slot < maxTexCount);
#endif
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texID);
}

void RenderingWorker::drawDrawable(Drawable const& d, Mat4 const& viewProjectionMatrix, Mat4 const& pixelProjectionMatrix)
{
	int texSlot{ 0 };

	Vertex vertecies[4];
	generateVertices(d, texSlot, viewProjectionMatrix, pixelProjectionMatrix, vertecies);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * MAX_VERTEX_COUNT, spriteShaderVBORaw);


	if (d.texRef.has_value()) {
		if (texCache.isTextureLoaded(d.texRef.value())) {
			bindTexture(d.texRef.value().id, texSlot);
		}
		else {
			bindTexture(TEXTURE_DEFAULT, texSlot);
		}
	}
	else {
		texSlot = -1;
	}

	// give the shader the possible texture slots
	int texSamplers[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	glUniform1iv(50, 32, texSamplers);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLES, 1, 4);
}

void RenderingWorker::drawLayer(RenderLayer& layer, Mat4 const& cameraViewProj, Mat4 const& pixelProjectionMatrix)
{
	glClear(GL_DEPTH_BUFFER_BIT);

	if (layer.bSortForDepth) {
		if (layer.bStableSort) {
			std::stable_sort(layer.getDrawables().begin(), layer.getDrawables().end());
		}
		else {
			std::sort(layer.getDrawables().begin(), layer.getDrawables().end());
		}
	}

	int lastIndex = 0;
	while (lastIndex != layer.getDrawables().size()) {
		lastIndex = drawBatch(layer.getDrawables(), cameraViewProj, pixelProjectionMatrix, lastIndex);
		data->drawCallCount += 1;
	}
}
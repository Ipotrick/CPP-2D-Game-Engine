#include "RenderingWorker.h"

#include <sstream>
#include <fstream>

#include "Timing.h"

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
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

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

	auto vertexShader = readShader(vertexShaderPath);
	auto fragmentShader = readShader(fragmentShaderPath);
	shader = createShader(vertexShader, fragmentShader);

	auto vertexShadowShader = readShader(vertexShadowShaderPath);
	auto fragmentShadowShader = readShader(fragmentShadowShaderPath);
	shadowShader = createShader(vertexShadowShader, fragmentShadowShader);

	// enable blending when drawing a fragment twice
	glEnable(GL_BLEND);
	// change blending type to transparency blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	texHandler.initialize();
	
	verteciesRawBuffer = (float*)malloc(sizeof(Vertex) * maxVertexCount);
	indices = (uint32_t*)malloc(sizeof(uint32_t) * maxIndicesCount);

	int quadCount = 0;
	for (int i = 0; i < maxIndicesCount; i += 6) {
		indices[i + 0] = 0 + quadCount * 4; indices[i + 1] = 1 + quadCount * 4; indices[i + 2] = 2 + quadCount * 4;
		indices[i + 3] = 1 + quadCount * 4; indices[i + 4] = 2 + quadCount * 4; indices[i + 5] = 3 + quadCount * 4;
		quadCount++;
	}

	glGenBuffers(1, &verteciesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verteciesBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maxVertexCount, nullptr, GL_DYNAMIC_DRAW);
	// positions (2 float)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	// vertex color (4 float)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));
	// texture uv coordinates (2 float)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoord));
	// texture slot (1 int) 
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texID));
	// circle rendering mode enable
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, circle));
}

void RenderingWorker::operator()()
{
	initiate();
	
	while (data->run) {
		auto& drawables = data->renderBuffer->drawables;
		auto& camera = data->renderBuffer->camera;
		{	
			Timer<> t(data->new_renderTime);
			// refresh texture Refs
			texHandler.refreshRefMap(data->renderBuffer->newTextureRefs);

			mat3 viewProjectionMatrix = mat3::scale(camera.zoom) * mat3::scale(camera.frustumBend) * mat3::rotate(-camera.rotation) * mat3::translate(-camera.position);

			std::sort(drawables.begin(), drawables.end(),
				[](Drawable const& a, Drawable const& b) {
					return a.drawingPrio < b.drawingPrio;
				}
			);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			int lastIndex{ 0 };
			while (lastIndex != drawables.size()) {
				lastIndex = drawBatch(drawables, viewProjectionMatrix, lastIndex);
			}

			{	// push rendered image into image buffer
				std::lock_guard<std::mutex> l(window->mut);
				glfwSwapBuffers(window->glfwWindow);
			}
		}

		{	// process render side events
			{	// window
				std::lock_guard<std::mutex> l(window->mut);
				int width, height;
				glfwGetWindowSize(window->glfwWindow, &width, &height);
				glViewport(0, 0, window->width, window->height);
				window->width = width;
				window->height = height;
			}
		}

		{	// wait for main
			Timer<> t(data->new_renderSyncTime);
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

std::array<Vertex, 4> RenderingWorker::generateVertices(Drawable const& d, float texID, mat3 const& viewProjMat) {
	vec2 minTex{ 0,0 };
	vec2 maxTex{ 1,1 };
	if (texHandler.hasTexture(d.id) && texHandler.isTextureLoaded(texHandler.getTexRef(d.id).textureName)) {
		minTex = texHandler.getTexRef(d.id).minPos;
		maxTex = texHandler.getTexRef(d.id).maxPos;
	}

	bool isCircle = d.form ==  Form::CIRCLE ? 1.0f : 0.0f;

	mat3 modelMatrix2 = mat3::translate(vec2(d.position.x, d.position.y)) * mat3::rotate(d.rotation) * mat3::scale(vec2(d.scale.x, d.scale.y));
	if (!d.isInWindowSpace()) {
		modelMatrix2 = viewProjMat * modelMatrix2;
	}

	Vertex v1;
	v1.position = modelMatrix2 * vec2{ -0.5f, 0.5f };
	v1.texCoord = { minTex.x, maxTex.y };
	v1.color = d.color;
	v1.texID = texID;
	v1.circle = isCircle;
	Vertex v2;
	v2.position = modelMatrix2 * vec2{ 0.5f, 0.5f };
	v2.texCoord = { maxTex.x, maxTex.y };
	v2.color = d.color;
	v2.texID = texID;
	v2.circle = isCircle;
	Vertex v3;
	v3.position = modelMatrix2 * vec2{ -0.5f, -0.5f };
	v3.texCoord = { minTex.x, minTex.y };
	v3.color = d.color;
	v3.texID = texID;
	v3.circle = isCircle;
	Vertex v4;
	v4.position = modelMatrix2 * vec2{ 0.5f, -0.5f };
	v4.texCoord = { maxTex.x, minTex.y };
	v4.color = d.color;
	v4.texID = texID;
	v4.circle = isCircle;

	return { v1, v2, v3, v4 };
}



size_t RenderingWorker::drawBatch(std::vector<Drawable>& drawables, mat3 const& viewProjectionMatrix, size_t startIndex)
{
	glUseProgram(shader);
	glBindBuffer(GL_ARRAY_BUFFER, verteciesBuffer);

	// give the shader the possible texture slots
	int texSamplers[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	glUniform1iv(50, 32, texSamplers);

	bindTexture("white", 0);	// texture slot 0 is allways the white texture
	bindTexture("default", 1);

	uint32_t nextTextureSamplerSlot{ 2 }; // when there are no texture slots left, the batch is full and will be rendered.
	robin_hood::unordered_map<std::string_view, uint32_t> usedTexturesSamplerSlots;	// when a texture is used it will get a slot 
	// fill batch with vertices
	size_t index{ startIndex };
	int drawableCount{ 0 };
	for (; nextTextureSamplerSlot < 32 
		&& index < drawables.size() 
		&& drawableCount < maxRectCount; index++, drawableCount++)
	{
		Drawable const& d = drawables[index];
		uint32_t drawableSamplerSlot{ 0 };

		// check if drawable has texture
		if (texHandler.hasTexture(d.id)) {
			if (texHandler.isTextureLoaded(texHandler.getTexRef(d.id).textureName)) {
				// is texture allready in the usedSamplerMap?
				if (usedTexturesSamplerSlots.contains(texHandler.getTexRef(d.id).textureName)) {
					// then just use the allready sloted texture sampler:
					drawableSamplerSlot = usedTexturesSamplerSlots[texHandler.getTexRef(d.id).textureName];
				}
				else {
					// add texture to usedSamplerSlotMap:
					usedTexturesSamplerSlots.insert({ texHandler.getTexRef(d.id).textureName , nextTextureSamplerSlot });
					// bind texture to sampler slot
					bindTexture(texHandler.getTexRef(d.id).textureName, nextTextureSamplerSlot);
					drawableSamplerSlot = nextTextureSamplerSlot;
					nextTextureSamplerSlot++;
				}
			}
			else {
				// drawable gets the error texture
				drawableSamplerSlot = 1;
			}
		}
		else {
			// drawable has no texture so it gets sampler slot 0 (white texture)
			drawableSamplerSlot = 0;
		}

		auto vertecies = generateVertices(d, drawableSamplerSlot, viewProjectionMatrix);
		// push vertex data in the rawBuffer
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < Vertex::floatCount; j++) {
				verteciesRawBuffer[(i * Vertex::floatCount + j) + (drawableCount * 4 * Vertex::floatCount)] = vertecies[i][j];
			}
		}
	}
	// push vertex data to the gpu
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * maxVertexCount, verteciesRawBuffer);

	// render drawableCount amount of drawables with glDrawElements
	glDrawElements(GL_TRIANGLES, drawableCount * 6, GL_UNSIGNED_INT, indices);
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

void RenderingWorker::bindTexture(std::string_view name, int slot)
{
#ifdef _DEBUG
	int maxTexCount;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexCount);
	assert(slot < maxTexCount);
#endif
	assert(texHandler.isTextureLoaded(name)); 
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texHandler.getTexture(name).openglTexID);
}

void RenderingWorker::drawDrawable(Drawable const& d, mat4 const& viewProjectionMatrix)
{
	constexpr int texSlot{ 0 };

	auto vertecies = generateVertices(d, texSlot, viewProjectionMatrix);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < Vertex::floatCount; j++) {
			verteciesRawBuffer[i * Vertex::floatCount + j] = vertecies[i][j];
		}
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * maxVertexCount, verteciesRawBuffer);


	if (texHandler.hasTexture(d.id)) {
		if (texHandler.isTextureLoaded(texHandler.getTexRef(d.id).textureName)) {
			bindTexture(texHandler.getTexRef(d.id).textureName, texSlot);
		}
		else {
			bindTexture("default", texSlot);
		}
	}
	else {
		bindTexture("white", texSlot);
	}

	// give the shader the possible texture slots
	int texSamplers[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	glUniform1iv(50, 32, texSamplers);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLES, 1, 4);
}
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
}

void RenderingWorker::operator()()
{
	initiate();
	
	float positions[16] =
	{
		-0.5, 0.5, 0.0f, 1.0f,
		0.5,0.5, 1.0f, 1.0f,
		-0.5,-0.5, 0.0f, 0.0f,
		0.5,-0.5, 1.0f, 0.0f
	};
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const void*)(sizeof(float) * 2));
	
	while (data->run) {
		auto& drawables = data->renderBuffer->drawables;
		auto& camera = data->renderBuffer->camera;
		{	
			Timer<> t(data->new_renderTime);
			// add texture Refs
			textureRefMap.clear();
			for (auto& tex : data->renderBuffer->newTextureRefs) {
				if (!texHandler.isTextureLoaded(tex.second.textureName)) {
					// load all not loaded  textures from file
					auto success = texHandler.loadTexture(tex.second.textureName);
					assert(success);
				}
				textureRefMap.insert({ tex.first, tex.second });
			}

			mat4 viewProjectionMatrix = mat4::scale(camera.zoom) * mat4::scale(camera.frustumBend) * mat4::rotate_z(-camera.rotation) * mat4::translate(-camera.position);

			std::sort(drawables.begin(), drawables.end(),
				[](Drawable const& a, Drawable const& b) {
					return a.drawingPrio < b.drawingPrio;
				}
			);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(shader);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			for (auto const& drawable : drawables) {
				if (drawable.isInWindowSpace()) {
					drawDrawable(drawable, mat4::identity());
				}
				else {
					drawDrawable(drawable, viewProjectionMatrix);
				}
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

void RenderingWorker::drawDrawable(Drawable const& d, mat4 const& viewProjectionMatrix)
{
	constexpr int texSlot{ 0 };
	if (textureRefMap.contains(d.id)) {
		if (texHandler.isTextureLoaded(textureRefMap[d.id].textureName)) {
			glUniform1i(10, texSlot);	// set unifrom of texture to slot 0
			bindTexture(textureRefMap[d.id].textureName, texSlot);
		}
		else { 
			glUniform1i(10, texSlot);	// set unifrom of texture to slot 0
			bindTexture("default", texSlot);
		}
	}
	else {
		glUniform1i(10, texSlot);	// set unifrom of texture to slot 0
		bindTexture("white", texSlot);
	}

	mat4 modelMatrix = mat4::translate(vec3(d.position.x, d.position.y, 1 - d.drawingPrio)) * mat4::rotate_z(d.rotation) * mat4::scale(vec3(d.scale.x, d.scale.y, 1));
	glUniformMatrix4fv(3, 1, GL_FALSE, modelMatrix.data());
	glUniformMatrix4fv(4, 1, GL_FALSE, viewProjectionMatrix.data());
	glUniform4fv(5, 1, d.color.data());
	glUniform1i(6, (d.form == Form::CIRCLE ? 1 : 0));
	glUniform2fv(7, 1, d.position.data());
	glUniform1f(8, d.scale.r / 2.0f);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLES, 1, 4);
}

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

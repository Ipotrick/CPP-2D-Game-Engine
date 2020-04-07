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
}

void RenderingWorker::operator()()
{
	initiate();

	float positions[8] =
	{
		-0.5,0.5,
		0.5,0.5,
		-0.5,-0.5,
		0.5,-0.5
	};
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	auto& worldDrawables = data->renderBuffer.worldSpaceDrawables;
	auto& windowDrawables = data->renderBuffer.windowSpaceDrawables;
	auto& camera = data->renderBuffer.camera;
	while (data->run) {
		{	// wait for main
			Timer<> t(data->new_renderSyncTime);
			std::unique_lock<std::mutex> switch_lock(data->mut);
			data->ready = true;
			data->cond.notify_one();
			data->cond.wait(switch_lock, [&]() { return data->ready == false; });
		}

		{	// process renderdata
			Timer<> t(data->new_renderTime);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			mat4 viewProjectionMatrix = mat4::scale(camera.zoom) * mat4::scale(camera.frustumBend) * mat4::rotate_z(-camera.rotation) * mat4::translate(-camera.position);

			std::sort(worldDrawables.begin(), worldDrawables.end(),
				[](Drawable const& a, Drawable const& b) {
					return a.drawingPrio < b.drawingPrio;
				}
			);
			std::sort(windowDrawables.begin(), windowDrawables.end(),
				[](Drawable const& a, Drawable const& b) {
					return a.drawingPrio < b.drawingPrio;
				}
			);

			//render game objects that can be lit and/or shadowed (drawprio <0.9)
			glUseProgram(shader);
			/*glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);*/
			auto iterWorld = worldDrawables.begin();
			auto iterWindow = windowDrawables.begin();
			while (!(iterWorld == worldDrawables.end()) || !(iterWindow == windowDrawables.end())) {
				if (!(iterWorld == worldDrawables.end()) && !(iterWindow == windowDrawables.end())) {	// there are world AND window drawables left
					if (iterWorld->drawingPrio < iterWindow->drawingPrio) {	// decide wich to draw considering drawingprio
						if (iterWorld->drawingPrio >= 0.9f) break;
						drawWorldSpace(*iterWorld++, viewProjectionMatrix);
					}
					else {
						if (iterWindow->drawingPrio >= 0.9f) break;
						drawWindowSpace(*iterWindow++);
					}
				}
				else if (!(iterWorld == worldDrawables.end())) {	// there are world drawables left
					if (iterWorld->drawingPrio >= 0.9f) break;
					drawWorldSpace(*iterWorld++, viewProjectionMatrix);
				}
				else {	// there are window drawables left
					if (iterWindow->drawingPrio >= 0.9f) break;
					drawWindowSpace(*iterWindow++);
				}
			}

			// render lightmap
			glUseProgram(shadowShader);
			// TODO: change blending modus to additive
			// TODO: change active framebuffer to lightbuffer

			// TODO: loop: for every light
				// TODO: compute visibility area
				// TODO: render visibility area for sencil buffer test
				// TODO: render light into light buffer WITH sencil buffer test to not generate light in shadow

			// TODO: add ambient light to the light framebuffer

			// TODO: blend(multiply) light buffer with rendered world buffer

			// render game objects that can NOT be lit and/or shadowed (drawprio >= 0.9)
			// TODO: change active framebuffer to mainbuffer
			// TODO: change blending back to transparency blending
			glUseProgram(shader);
			while (!(iterWorld == worldDrawables.end()) || !(iterWindow == windowDrawables.end())) {
				if (!(iterWorld == worldDrawables.end()) && !(iterWindow == windowDrawables.end())) {	// there are world AND window drawables left
					if (iterWorld->drawingPrio > iterWindow->drawingPrio) {	// decide wich to draw considering drawingprio
						drawWorldSpace(*iterWorld++, viewProjectionMatrix);
					}
					else {
						drawWindowSpace(*iterWindow++);
					}
				}
				else if (!(iterWorld == worldDrawables.end())) {	// there are world drawables left
					drawWorldSpace(*iterWorld++, viewProjectionMatrix);
				}
				else {	// there are window drawables left
					drawWindowSpace(*iterWindow++);
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

				if (glfwWindowShouldClose(window->glfwWindow)) {
					data->run = false;
				}
			}
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

void RenderingWorker::drawWorldSpace(Drawable const& d, mat4 const& viewProjectionMatrix)
{
	mat4 modelMatrix = mat4::translate(vec3(d.position.x, d.position.y, 1 - d.drawingPrio)) * mat4::rotate_z(d.rotation) * mat4::scale(vec3(d.scale.x, d.scale.y, 1));
	glUniformMatrix4fv(1, 1, GL_FALSE, modelMatrix.data());
	glUniformMatrix4fv(6, 1, GL_FALSE, viewProjectionMatrix.data());
	glUniform4fv(2, 1, d.color.data());
	glUniform1i(3, (d.form == Form::CIRCLE ? 1 : 0));
	glUniform2fv(4, 1, d.position.data());
	glUniform1f(5, d.scale.r / 2.0f);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLES, 1, 4);
}

void RenderingWorker::drawWindowSpace(Drawable const& d)
{
	mat4 modelMatrix = mat4::translate(d.position) * mat4::rotate_z(d.rotation) * mat4::scale(vec3(d.scale));
	glUniformMatrix4fv(1, 1, GL_FALSE, (modelMatrix).data());
	glUniformMatrix4fv(6, 1, GL_FALSE, mat4::identity().data());
	glUniform4fv(2, 1, d.color.data());
	glUniform1i(3, (d.form == Form::CIRCLE ? 1 : 0));
	glUniform2fv(4, 1, d.position.data());
	glUniform1f(5, d.scale.r / 2.0f);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLES, 1, 4);
}
#include "Renderer.h"

void Renderer::initiate()
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

void Renderer::operator()()
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

	auto& lights = sharedData->lights;
	auto& worldDrawables = sharedData->renderBufferB.worldSpaceDrawables;
	auto& windowDrawables = sharedData->renderBufferB.windowSpaceDrawables;
	auto& camera = sharedData->renderBufferB.camera;
	while (sharedData->run) {
		{	// wait for main
			Timer<> t(sharedData->new_renderSyncTime);
			std::unique_lock<std::mutex> switch_lock(sharedData->mut);
			sharedData->ready = true;
			sharedData->cond.notify_one();
			sharedData->cond.wait(switch_lock, [&]() { return sharedData->ready == false; });
		}

		{	// process renderdata
			Timer<> t(sharedData->new_renderTime);
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
					if (iterWorld->drawingPrio > iterWindow->drawingPrio) {	// decide wich to draw considering drawingprio
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
					sharedData->run = false;
				}
			}
		}
	}

	end();
}

void Renderer::end() {
	std::unique_lock<std::mutex> switch_lock(sharedData->mut);
	sharedData->ready = true;
	sharedData->run = false;
	sharedData->cond.notify_one();
}

std::string Renderer::readShader(std::string path_)
{
	std::stringstream ss;
	std::ifstream ifs(path_);
	if (!ifs.good()) {
		std::cerr << "error: could not read shader from path: " << path_ << std::endl;
		sharedData->run = false;
	}
	std::string line;
	while (getline(ifs, line)) {
		ss << line << '\n';
	}
	return ss.str();
}

void Renderer::drawWorldSpace(Drawable const& d, mat4 const& viewProjectionMatrix)
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

void Renderer::drawWindowSpace(Drawable const& d)
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

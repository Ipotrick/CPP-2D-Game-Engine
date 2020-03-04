#include "Renderer.h"

void Renderer::initiate()
{
	
	int err = initGLWindow();
	if (err != 0)	std::cerr << "error: terminated cause of error in window creation, error code: " << err << std::endl;

	auto vertexShader = readShader(vertexShaderPath);
	auto fragmentShader = readShader(fragmentShaderPath);

	shader = createShader(vertexShader, fragmentShader);
	glUseProgram(shader);
}

int Renderer::initGLWindow() {
	std::lock_guard<std::mutex> l(window->mut);
	if (!glfwInit()) {
		return -1;
	}
	else {
		glWindow = glfwCreateWindow(window->width, window->height, window->name.c_str(), nullptr, nullptr);
		if (!glWindow) {
			glfwTerminate();
			return -2;
		}
		else {
			glfwMakeContextCurrent(glWindow);

			if (glewInit() != GLEW_OK) {
				glfwTerminate();
				return -3;
			}
		}
	}
	return 0;
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

	auto& drawables = sharedData->renderBufferB.drawables;
	auto& camera = sharedData->renderBufferB.camera;
	while (sharedData->run && !glfwWindowShouldClose(glWindow)) {
		{	/* process non render inputs */
			{	/* window */
				std::lock_guard<std::mutex> l(window->mut);
				glfwSetWindowSize(glWindow, window->width, window->height);
				glViewport(0, 0, window->width, window->height);
			}
		}
		
		{	/* process renderdata */
			Timer<> t(sharedData->new_renderTime);

			std::sort(sharedData->renderBufferB.drawables.begin(), sharedData->renderBufferB.drawables.end(),
				[](Drawable a, Drawable b) {
					return a.position.z < b.position.z;
				}
			);

			/* TODO generate viewProjection matrix from camera */
			mat4 viewProjectionMatrix = mat4::translate(camera.position) * mat4::scale(camera.zoom) * mat4::scale(-camera.frustumBend) * mat4::rotate_z(-camera.rotation);

			glClear(GL_COLOR_BUFFER_BIT);

			for (auto& el : drawables) {

				mat4 modelMatrix = mat4::translate(el.position) * mat4::rotate_z(el.rotation) * mat4::scale(vec3(el.scale));
				glUniformMatrix4fv(1, 1, GL_FALSE, (viewProjectionMatrix * modelMatrix).data());
				glUniform4fv(2, 1, el.color.data());
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glDrawArrays(GL_TRIANGLES, 1, 4);
			}

			glfwSwapBuffers(glWindow);
		}
		
		{	/* process render side events events */
			{	/* window */
				std::lock_guard<std::mutex> l(window->mut);
				glfwPollEvents();
				int width, height;
				glfwGetWindowSize(glWindow, &width, &height);
				glViewport(0, 0, window->width, window->height);
				window->width = width;
				window->height = height;
			}
		}

		{	/* wait for main */
			Timer<> t(sharedData->new_renderSyncTime);
			std::unique_lock<std::mutex> switch_lock(sharedData->mut);
			sharedData->ready = true;
			sharedData->cond.notify_one();
			sharedData->cond.wait(switch_lock, [&]() { return sharedData->ready == false; });
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

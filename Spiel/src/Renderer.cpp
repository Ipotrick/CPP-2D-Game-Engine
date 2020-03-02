#include "Renderer.h"

void Renderer::initiate()
{
	std::lock_guard<std::mutex> l(window->mut);
	int err = window->init();
	if (err != 0)	std::cerr << "error: terminated cause of error in window creation, error code: " << err << std::endl;

	auto vertexShader = readShader(vertexShaderPath);
	auto fragmentShader = readShader(fragmentShaderPath);

	shader = createShader(vertexShader, fragmentShader);
	glUseProgram(shader);
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

	while (sharedData->run) {
		{
			sharedData->new_renderTime = std::chrono::microseconds(0);
			Timer<> t(sharedData->new_renderTime);
			{
				std::lock_guard<std::mutex> l(window->mut);
				auto& drawables = sharedData->renderBufferB.drawables;
				auto& camera = sharedData->renderBufferB.camera;

				glClear(GL_COLOR_BUFFER_BIT);

				for (auto& el : drawables) {

					mat4 modelMatrix = mat4::translate(el.position) * mat4::rotate_z(el.rotation) * mat4::scale(vec3(el.scale));
					GLint loc = glGetUniformLocation(shader, "modelMatrix");
					glUniformMatrix4fv(loc, 1, GL_FALSE, modelMatrix.data());
					glDrawArrays(GL_TRIANGLES, 0, 3);
					glDrawArrays(GL_TRIANGLES, 1, 4);
				}

				glfwSwapBuffers(window->glWindow);

				glfwPollEvents();
			}
			
		}

		{	/* wait for main */
			std::unique_lock<std::mutex> switch_lock(sharedData->mut);
			sharedData->ready = true;
			sharedData->cond.notify_one();
			sharedData->cond.wait(switch_lock, [&]() { return sharedData->ready == false; });
		}
	}
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

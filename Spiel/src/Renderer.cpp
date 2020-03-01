#include "Renderer.h"

void Renderer::initiate()
{
	std::lock_guard<std::mutex> l(window->mut);
	int err = window->init();
	if (err != 0)	std::cerr << "error: terminated cause of error in window creation, error code: " << err << std::endl;

	shader = createShader(vertexShader, fragmentShader);
	glUseProgram(shader);
}

void Renderer::operator()()
{
	initiate();

	float positions[6] =
	{
		0.0, 0.5,
		-0.5, -0.5,
		0.5, -0.5
	};

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

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

				glDrawArrays(GL_TRIANGLES, 0, 3);

				glfwSwapBuffers(window->glWindow);
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
#include "Engine.hpp"

using namespace std::literals::chrono_literals;

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_)
{
	std::cout << "waiter" << std::endl;
	/*
	* there can only be one engine instance at a time
	*/
	if (bInstantiated) {
		std::cerr << "ERRROR: there can only be one instance of an engine at a time!" << std::endl;
		exit(-1);
	}
	bInstantiated = true;

	running = true;
	iteration = 0;
	minimunLoopTime = 1ms;	// 10000 microseconds = 10 milliseonds => 100 loops per second
	maxDeltaTime = 0.02f;
	deltaTime = 0.0;
	window.open(windowName_, windowWidth_, windowHeight_);
	renderer.initialize(&window);
	running = true;

	perfLog.submitTime("maintime");
	perfLog.submitTime("mainwait");
	perfLog.submitTime("updatetime");
	perfLog.submitTime("physicstime");
	perfLog.submitTime("physicsprepare");
	perfLog.submitTime("physicscollide");
	perfLog.submitTime("physicsexecute");
	perfLog.submitTime("rendertime");
	perfLog.submitTime("calcRotaVecTime");
	deltaTimeQueue.push_back(maxDeltaTime);

	uiContext.recursionDepth = 1;
	uiContext.drawMode = RenderSpace::PixelSpace;
	uiContext.scale = 1.0f;
	uiContext.ulCorner = { 0.0f, static_cast<float>(window.height) };
	uiContext.drCorner = { static_cast<float>(window.width), 0.0f };
}

Engine::~Engine()
{
	/*
	* there can only be one engine instance at a time
	*/
	if (!bInstantiated) {
		std::cerr << "ERRROR: there can only be one instance of an engine at a time!" << std::endl;
		exit(-1);
	}
	bInstantiated = false;

	renderer.reset();
	window.close();
}

float Engine::getDeltaTime(int sampleSize)
{
	if (sampleSize == 1) {
		return deltaTimeQueue[0];
	}
	else {
		int maxSamples = std::min(deltaTimeQueue.size(), 100ui64);

		sampleSize = std::min(sampleSize, maxSamples);
		float sum{ 0.0f };
		for (int i = 0; i < sampleSize; ++i) {
			sum += deltaTimeQueue[i];
		}
		return sum / sampleSize;
	}
}

Vec2 Engine::getWindowSize() {
	std::lock_guard<std::mutex> l(window.mut);
	return { static_cast<float>(window.width), static_cast<float>(window.height) };
}

float Engine::getWindowAspectRatio() {
	std::lock_guard<std::mutex> l(window.mut);
	return static_cast<float>(window.width)/ static_cast<float>(window.height);
}

void Engine::run() {
	create();

	for(iteration = 0; running; ++iteration) {
		Timer loopTimer(new_deltaTime);
		Waiter loopWaiter(minimunLoopTime, Waiter::Type::BUSY);
		deltaTime = micsecToFloat(new_deltaTime);
		if (deltaTimeQueue.size() > 99) {
			deltaTimeQueue.pop_back();
		}
		deltaTimeQueue.push_front(deltaTime);

		perfLog.commitTimes();
		{
			Timer mainTimer(perfLog.getInputRef("maintime"));

			update(getDeltaTimeSafe());

			// update in:
			in.engineUpdate(renderer.getCamera());

			// update rendering:
			{
				LogTimer t(std::cout, "time taken for ui update: ");
				ui.update();
			}
			{
				LogTimer t(std::cout, "time taken for ui draw: ");
				ui.draw(uiContext);
			}
			renderer.waitTillFinished();
		}
		if (glfwWindowShouldClose(window.glfwWindow)) { // if window closes the program ends
			running = false;
			break;
		}
		if (!glfwGetWindowAttrib(window.glfwWindow, GLFW_FOCUSED) && in.getFocus() != Focus::Out) {
			in.takeFocus(Focus::Out);
			in.takeMouseFocus(Focus::Out);
		}
		else if (glfwGetWindowAttrib(window.glfwWindow, GLFW_FOCUSED) && in.getFocus() == Focus::Out) {
			in.returnFocus();
			in.returnMouseFocus();
		}
		renderer.startRendering();
	}
	renderer.reset();

	destroy();
}
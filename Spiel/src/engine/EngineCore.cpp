#include "EngineCore.hpp"

using namespace std::literals::chrono_literals;

EngineCore::EngineCore()
{
	/*
	* there can only be one engine instance at a time
	*/
	if (bInstantiated) {
		std::cerr << "ERRROR: there can only be one instance of an engine at a time!" << std::endl;
		exit(-1);
	}
	bInstantiated = true;
}

EngineCore::~EngineCore()
{
	/*
	* there can only be one engine instance at a time
	*/
	if (!bInstantiated) {
		std::cerr << "ERRROR: there can only be one instance of an engine at a time!" << std::endl;
		exit(-1);
	}
	bInstantiated = false;
}

void EngineCore::initialize(std::string windowName, uint32_t windowWidth, uint32_t windowHeight)
{
	iteration = 0;
	minimunLoopTime = 0ms;	// 10000 microseconds = 10 milliseonds => 100 loops per second
	maxDeltaTime = 0.02f;
	deltaTime = 0.0;
	window.open(windowName, windowWidth, windowHeight);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	renderer.initialize(&window);
	running = true;
	deltaTimeQueue.push_back(maxDeltaTime);
	JobSystem::initialize();

	uiContext.recursionDepth = 1;
	uiContext.drawMode = RenderSpace::PixelSpace;
	uiContext.scale = 1.0f;
	uiContext.ulCorner = { 0.0f, static_cast<float>(window.height) };
	uiContext.drCorner = { static_cast<float>(window.width), 0.0f };
}

float EngineCore::getDeltaTime(int sampleSize)
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

Vec2 EngineCore::getWindowSize() {
	std::lock_guard<std::mutex> l(window.mut);
	return { static_cast<float>(window.width), static_cast<float>(window.height) };
}

float EngineCore::getWindowAspectRatio() {
	std::lock_guard<std::mutex> l(window.mut);
	return static_cast<float>(window.width)/ static_cast<float>(window.height);
}

void EngineCore::run() {
	running = true;
	create();

	for(iteration = 0; running; ++iteration) {
		Timer loopTimer(new_deltaTime);							// messures time taken for frame
		Waiter loopWaiter(minimunLoopTime, Waiter::Type::BUSY);	// makes sure the loop will take a specified minimum amount of time

		deltaTime = micsecToFloat(new_deltaTime);
		totalDeltaTime += deltaTime;
		if (deltaTimeQueue.size() > 99) {
			deltaTimeQueue.pop_back();
		}
		deltaTimeQueue.push_front(deltaTime);

		update(getDeltaTimeSafe());

		// update in:
		in.engineUpdate(renderer.getCamera());

		// update rendering:
		ui.update();
		ui.draw(uiContext);
		renderer.waitTillFinished();

		if (!glfwGetWindowAttrib(window.glfwWindow, GLFW_FOCUSED) && in.getFocus() != Focus::Out) {
			in.takeFocus(Focus::Out);
			in.takeMouseFocus(Focus::Out);
		}
		else if (glfwGetWindowAttrib(window.glfwWindow, GLFW_FOCUSED) && in.getFocus() == Focus::Out) {
			in.returnFocus();
			in.returnMouseFocus();
		}
		// update the uiContext while window acces is mutex free:
		uiContext.ulCorner = { 0.0f, static_cast<float>(window.height) };
		uiContext.drCorner = { static_cast<float>(window.width), 0.0f };

		if (glfwWindowShouldClose(window.glfwWindow)) { // if window closes the program ends
			running = false;
			break;
		}
		else {
			renderer.startRendering();
		}

	}
	renderer.reset();

	window.close();

	destroy();
}
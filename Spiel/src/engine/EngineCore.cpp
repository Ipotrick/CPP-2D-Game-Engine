#include "EngineCore.hpp"

using namespace std::literals::chrono_literals;

EngineCore::EngineCore()
{
}

EngineCore::~EngineCore()
{
}

void EngineCore::initialize(std::string windowName, uint32_t windowWidth, uint32_t windowHeight)
{
	iteration = 0;
	minimunLoopTime = 0ms;	// 10000 microseconds = 10 milliseonds => 100 loops per second
	maxDeltaTime = 0.02f;
	deltaTime = 0.0;
	if (!window.open(windowName, windowWidth, windowHeight)) {
		std::cerr << "errror: could not open window!" << std::endl;
		exit(-1);
	}
	renderer.initialize(&window);
	running = true;
	deltaTimeQueue.push_back(maxDeltaTime);

	//uiContext = {
	//	.ulCorner = { 0.0f, static_cast<float>(window.getHeight()) },
	//	.drCorner = { static_cast<float>(window.getWidth()), 0.0f },
	//	.scale = 1.0f,
	//	.recursionDepth = 1,
	//	.drawMode = RenderSpace::PixelSpace
	//};
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
	return { static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()) };
}

float EngineCore::getWindowAspectRatio() {
	return static_cast<float>(window.getWidth())/ static_cast<float>(window.getHeight());
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
		in.engineUpdate();

		// update rendering:
		//ui.update();
		//ui.draw(uiContext);
		renderer.waitTillFinished();

		if (!window.isFocused() && in.getFocus() != Focus::Out) {
			in.takeFocus(Focus::Out);
			in.takeMouseFocus(Focus::Out);
		}
		else if (window.isFocused() && in.getFocus() == Focus::Out) {
			in.returnFocus();
			in.returnMouseFocus();
		}
		// update the uiContext while window acces is mutex free:
		//uiContext.ulCorner = { 0.0f, static_cast<float>(window.getHeight()) };
		//uiContext.drCorner = { static_cast<float>(window.getWidth()), 0.0f };

		if (window.shouldClose()) { // if window closes the program ends
			running = false;
			break;
		}
		else {
			renderer.render();
		}

	}
	renderer.reset();

	window.close();

	destroy();
}

void globalInitialize()
{
	JobSystem::initialize();
}

#include "EngineCore.hpp"

#include "rendering/RenderWorkerThread.hpp"

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
	if (!mainWindow.open(windowName, windowWidth, windowHeight)) {
		std::cerr << "errror: could not open window!" << std::endl;
		exit(-1);
	}
	renderer.initialize(&mainWindow);
	running = true;
	deltaTimeQueue.push_back(maxDeltaTime);
	bInitialized = true;

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
	return { static_cast<float>(mainWindow.getWidth()), static_cast<float>(mainWindow.getHeight()) };
}

float EngineCore::getWindowAspectRatio() {
	return static_cast<float>(mainWindow.getWidth())/ static_cast<float>(mainWindow.getHeight());
}

void EngineCore::run() {
	running = true;
	create();
	assert(bInitialized == true);

	for(iteration = 0; running; ++iteration) {
		Timer loopTimer(new_deltaTime);							// messures time taken for frame
		Waiter loopWaiter(minimunLoopTime, Waiter::Type::SLEEPY);	// makes sure the loop will take a specified minimum amount of time

		deltaTime = micsecToFloat(new_deltaTime);
		totalDeltaTime += deltaTime;
		if (deltaTimeQueue.size() > 99) {
			deltaTimeQueue.pop_back();
		}
		deltaTimeQueue.push_front(deltaTime);

		update(getDeltaTimeSafe());

		// update rendering:
		renderer.waitTillFinished();

		mainWindow.update();

		if (mainWindow.shouldClose()) { // if window closes the program ends
			running = false;
			break;
		}
		else {
			renderer.render();
		}

	}
	renderer.reset();

	mainWindow.close();

	destroy();
}

void globalInitialize()
{
	if (!glfwInit()) {
		std::cerr << "ERROR: failed to initialize GLEW!" << std::endl;
		exit(-1);
	}
	JobSystem::initialize();
	RenderWorkerThread::init();
}

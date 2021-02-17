#include "EngineCore.hpp"

using namespace std::literals::chrono_literals;

EngineCore::EngineCore(std::string windowName, uint32_t windowWidth, uint32_t windowHeight)
{
	iteration = 0;
	minimunLoopTime = 0ms;	// 10000 microseconds = 10 milliseonds => 100 loops per second
	maxDeltaTime = 0.02f;
	deltaTime = 0.0;
	if (!mainWindow.open(windowName, windowWidth, windowHeight)) {
		std::cerr << "errror: could not open window!" << std::endl;
		exit(-1);
	}
	running = true;
	deltaTimeQueue.push_back(maxDeltaTime);
	bInitialized = true;
}

EngineCore::~EngineCore()
{
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

		mainWindow.update(deltaTime);

		if (mainWindow.shouldClose()) { // if window closes the program ends
			running = false;
			break;
		}

	}

	destroy();
}

void globalInitialize()
{
	if (!glfwInit()) {
		std::cerr << "ERROR: failed to initialize GLEW!" << std::endl;
		exit(-1);
	}
	JobSystem::initialize();
}

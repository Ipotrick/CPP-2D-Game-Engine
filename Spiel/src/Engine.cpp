#include "Engine.h"

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_):
	running{ true },
	window(windowName_, windowWidth_, windowHeight_),
	iteration{ 0 }
{
	Collidable::initializeId();
}

Engine::~Engine() {

}

void Engine::commitTimeMessurements() {
	last_dTime = micsecToDouble(dTime);
	last_updateTime = micsecToDouble(updateTime);
	last_physicsTime = micsecToDouble(physicsTime);
	last_renderTime = micsecToDouble(renderTime);

	dTime = std::chrono::microseconds(0);
	updateTime = std::chrono::microseconds(0);
	physicsTime = std::chrono::microseconds(0);
	renderTime = std::chrono::microseconds(0);
}

void Engine::run() {
	create();

	while (running) {
		commitTimeMessurements();

		{
			Timer<> t(updateTime);
			update();
		}
		{
			Timer<> t(physicsTime);

		}
	}

	destroy();
}
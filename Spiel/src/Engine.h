#pragma once

#include <chrono>

#include "Timing.h"
#include "Window.h"
#include "Camera.h"
#include "Physics.h"

#include "Entity.h"

class Engine
{
public:
	Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_);
	~Engine();

	virtual void run() final;

	virtual void create() = 0;
	virtual void update() = 0;
	virtual void destroy() = 0;

					/*-- utility --*/
	/* returns time difference to last physics dispatch*/
	double getLastDTime() { return last_dTime; }
	/* returns time it took to process the last update task*/
	double getLastUpdateTime() { return last_updateTime; }
	/* returns time it took to process the last physics task */
	double getLastPhysicsTime() { return last_physicsTime; }
	/* returns time it took to render */
	double getLastRenderTime() { return last_renderTime; }
	/* returns the number of past iterations */
	uint32_t getIteration() { return iteration; }

	/* returns printable performance overview */
	std::string getPerfInfo(uint32_t grade);

public:
	Physics physics;
	Window window;
	Camera camera;

private:
	void commitTimeMessurements();

	bool running;

	std::chrono::microseconds dTime;
	double last_dTime;
	std::chrono::microseconds updateTime;
	double last_updateTime;
	std::chrono::microseconds physicsTime;
	double last_physicsTime;
	std::chrono::microseconds renderTime;
	double last_renderTime;

	uint32_t iteration;
};


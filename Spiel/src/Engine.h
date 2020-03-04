#pragma once

#include <chrono>
#include <iostream>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "input.h"
#include "Timing.h"
#include "Window.h"
#include "Camera.h"
#include "Physics.h"
#include "Entity.h"
#include "World.h"

#include "Renderer.h"

class Engine
{
public:
	Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_);
	~Engine();

	virtual void run() final;

	virtual void create() = 0;
	virtual void update(World& world, double deltaTime) = 0;
	virtual void destroy() = 0;

	virtual void physicsUpdate(World& world, double deltaTime) final;

					/*-- general statistics utility --*/
	/* returns time difference to last physics dispatch*/
	double getDeltaTime() { return deltaTime; }
	/* returns physics + update + bufferSwapTime */
	double getMainTime() { return mainTime; }
	/* returns time it took to process the last update task*/
	double getUpdateTime() { return updateTime; }
	/* returns time it took to process the last physics task */
	double getPhysicsTime() { return physicsTime; }
	/* returns time it took to render */
	double getRenderTime() { return renderTime; }
	/* returns time the main thread had to wait for the renderer thread */
	double getMainSyncTime() { return mainSyncTime; }
	/* returns the number of past iterations */
	uint32_t getIteration() { return iteration; }
	std::string getPerfInfo(int detail);

					/*-- input utility --*/
	/* returns the status(KEYSTATUS) of a given key_(KEY)  */
	KEYSTATUS getKeyStatus(KEY key_);
	/* returns if a given key_ is pressed */
	bool keyPressed(KEY key_);
	/* returns if a given key_ is released */
	bool keyReleased(KEY key_);
	/* returns if a given key_ is repeating */
	bool keyRepeating(KEY key_);

					/*-- window utility --*/
	/* returns size of window */
	vec2 getWindowSize();
	/* returns aspect ration width/height of the window*/
	float getWindowAspectRatio();

public:
	World world;
	Physics physics;
	Camera camera;

	std::chrono::microseconds minimunLoopTime;

private:
	void commitTimeMessurements();

private:
	bool running;
	uint32_t iteration;

	std::shared_ptr<Window> window;

	std::chrono::microseconds new_deltaTime;
	double deltaTime;
	std::chrono::microseconds new_mainTime;
	double mainTime;
	std::chrono::microseconds new_updateTime;
	double updateTime;
	std::chrono::microseconds new_physicsTime;
	double physicsTime;
	std::chrono::microseconds new_mainSyncTime;
	double mainSyncTime;
	std::chrono::microseconds new_mainWaitTime;
	double mainWaitTime;
	std::chrono::microseconds new_renderBufferPushTime;
	double renderBufferPushTime;
	std::chrono::microseconds new_renderTime;
	double renderTime;
	std::chrono::microseconds new_renderSyncTime;
	double renderSyncTime;
	
	std::shared_ptr<RendererSharedData> sharedRenderData;
	std::thread renderThread;
	RenderBuffer renderBufferA;
};


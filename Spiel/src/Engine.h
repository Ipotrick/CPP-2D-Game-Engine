#pragma once

#include <chrono>
#include <iostream>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

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

					/*-- utility --*/
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
	/*
		detail:
		0: deltaTime, ticks/s
		1: deltaTime, ticks/s, renderTime
	*/
	std::string getPerfInfo(int detail);

public:
	World world;
	Physics physics;
	Camera camera;

	std::shared_ptr<Window> window;

	std::chrono::microseconds minimunLoopTime;

private:
	void commitTimeMessurements();

private:
	bool running;
	uint32_t iteration;

	//RenderBuffer renderBufferA;

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


#pragma once

#include <chrono>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "robin_hood.h"

#include "util/utils.hpp"
#include "types/Timing.hpp"
#include "util/Perf.hpp"
#include "types/BaseTypes.hpp"
#include "rendering/RenderTypes.hpp"
#include "rendering/Window.hpp"
#include "rendering/Camera.hpp"
#include "io/InputManager.hpp"
#include "ui/UIManager.hpp"
#include "rendering/Renderer.hpp"
#include "JobManager.hpp"	// TODO REMOVE
#include "JobSystem.hpp"
#include "entity/EntityComponentManagerView.hpp"

class EngineCore {
public:
	EngineCore();
	~EngineCore();

	// creates window and initializes renderer
	void initialize(std::string windowName, uint32_t windowWidth, uint32_t windowHeight);

	/* ends programm after finishing the current frame */
	virtual void quit() final { running = false; }

	/* call run to start the loop */
	virtual void run() final;

	/* specify what happenes once for initialisation */
	virtual void create() = 0;
	/* specify what happenes every update tick */
	virtual void update(float deltaTime) = 0;
	/* specify what happenes once for destruction */
	virtual void destroy() = 0;

	/*-- general statistics utility --*/
/* returns time difference to last physics dispatch, O(1)*/
	static float getDeltaTime(int sampleSize = 1);
	static float getTotalDeltaTime() { return totalDeltaTime; }
	/* returns deltatime or the lowest allowed sim time difference, O(1)*/
	static float getDeltaTimeSafe(int sampleSize = 1) { return std::min(getDeltaTime(sampleSize), maxDeltaTime); }
	/* returns the number of past iterations , O(1)*/
	static uint32_t getIteration() { return iteration; }

	/*-- window utility --*/
/* returns size of window in pixel of your desktop resolution, O(1)*/
	static Vec2 getWindowSize();
	/* returns aspect ration width/height of the window, O(1)*/
	static float getWindowAspectRatio();

	inline static JobManager jobManager{ std::thread::hardware_concurrency() };


private:
	inline static Window window;
public:

	/*
	* singleton class used for rendering in the whole program
	*/
	inline static Renderer renderer;

	// Input
	inline static InputManager in{ window };

	// UI
	inline static UIContext uiContext;
	inline static UIManager ui{ renderer, in };

private:

	// meta
	inline static bool bInstantiated{ false }; // there can only be one active instance of the Engine as it is a singleton

	inline static bool running{ true };

	inline static uint32_t iteration{ 0 };

	inline static std::chrono::microseconds minimunLoopTime;

	inline static std::chrono::microseconds new_deltaTime;
	inline static float totalDeltaTime{ 0.0f };
	inline static float deltaTime;
	inline static float maxDeltaTime;
	inline static std::deque<float> deltaTimeQueue;

};
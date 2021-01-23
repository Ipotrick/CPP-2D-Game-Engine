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
#include "rendering/Renderer.hpp"
#include "JobSystem.hpp"
#include "entity/EntityComponentManagerView.hpp"

void globalInitialize();

class EngineCore {
public:
	EngineCore();
	~EngineCore();

	void operator=(const EngineCore& rhs) = delete;
	void operator=(EngineCore&& rhs) = delete;

	// creates window and initializes renderer
	void initialize(std::string windowName, uint32_t windowWidth, uint32_t windowHeight);

	/* ends programm after finishing the current frame */
	virtual void quit() final { running = false; }

	/* call run to start the loop */
	virtual void run() final;

	/* specify what happenes once for initialisation */
	virtual void create() = 0;
	/* specify what happenes once every update */
	virtual void update(float deltaTime) = 0;
	/* specify what happenes once for destruction */
	virtual void destroy() = 0;

	/**
	 * returns a smoothed delta time per frame in seconds.
	 * 
	 * \param sampleSize of past frames to calculate an average of elapsed time per frame
	 * \return deltatime elapsed time per frame, averaged over [sampleSize] samples
	 */
	float getDeltaTime(int sampleSize = 1);
	/**
	 * returns a smoothed delta time per frame in seconds.
	 * The deltatime has a maximal value, that is never overstepped.
	 * This is usefull for update loops that are sensitife to large time steps like physics.
	 * 
	 * \param sampleSize of past frames to calculate an average of elapsed time per frame
	 * \return deltatime elapsed time per frame, averaged over [sampleSize] samples
	 */
	float getDeltaTimeSafe(int sampleSize = 1) { return std::min(getDeltaTime(sampleSize), maxDeltaTime); }
	/**
	 * \return runtime of application in seconds.
	 */
	float getTotalDeltaTime() { return totalDeltaTime; }
	/**
	 * \return iteration of updates the applicatiuon is currently in.
	 */
	uint32_t getIteration() { return iteration; }

	void setMaxFPS(float fps)
	{
		minimunLoopTime = std::chrono::microseconds(cast<u64>(1.0f / fps * 1'000'000));
	}

	// TODO REPLACE WINDOWING AND CAMERA UTILITY:
	Vec2 getWindowSize();											// TODO REPLACE
	float getWindowAspectRatio();									// TODO REPLACE
	Window mainWindow;											// TODO REPLACE OR ENHANCE

	Renderer renderer;

private:
	bool running{ true };
	bool bInitialized{ false };

	uint32_t iteration{ 0 };

	std::chrono::microseconds minimunLoopTime;

	std::chrono::microseconds new_deltaTime;
	float totalDeltaTime{ 0.0f };
	float deltaTime;
	float maxDeltaTime;
	std::deque<float> deltaTimeQueue;
};
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
	static float getDeltaTime(int sampleSize = 1);
	/**
	 * returns a smoothed delta time per frame in seconds.
	 * The deltatime has a maximal value, that is never overstepped.
	 * This is usefull for update loops that are sensitife to large time steps like physics.
	 * 
	 * \param sampleSize of past frames to calculate an average of elapsed time per frame
	 * \return deltatime elapsed time per frame, averaged over [sampleSize] samples
	 */
	static float getDeltaTimeSafe(int sampleSize = 1) { return std::min(getDeltaTime(sampleSize), maxDeltaTime); }
	/**
	 * \return runtime of application in seconds.
	 */
	static float getTotalDeltaTime() { return totalDeltaTime; }
	/**
	 * \return iteration of updates the applicatiuon is currently in.
	 */
	static uint32_t getIteration() { return iteration; }

	// TODO REPLACE WINDOWING AND CAMERA UTILITY:
	static Vec2 getWindowSize();											// TODO REPLACE
	static float getWindowAspectRatio();									// TODO REPLACE
	inline static Window window;											// TODO REPLACE OR ENHANCE

	/**
	* singleton class used for rendering in the whole program
	*/
	inline static Renderer renderer;

	/**
	* singleton class used for all user input in the application
	*/
	inline static InputManager in{ window };

	/**
	* singleton class used for all graphical user interface
	*/
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
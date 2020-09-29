#pragma once

#include <chrono>
#include <iostream>
#include <sstream>
#include "std_extra.hpp"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

// makro definitions:
//#define DEBUG_STATIC_GRID
//#define DEBUG_QUADTREE
//#define DEBUG_QUADTREE2
//#define DEBUG_PATHFINDING

// ------------------

#include "robin_hood.h"

#include "Timing.hpp"
#include "Perf.hpp"
#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "QuadTree.hpp"
#include "Physics.hpp"
#include "input.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "EventHandler.hpp"
#include "World.hpp"
#include "UIManager.hpp"

// Core Systems
#include "MovementSystem.hpp"
#include "CollisionSystem.hpp"
#include "PhysicsSystem2.hpp"
#include "BaseSystem.hpp"
#include "Renderer.hpp"


class Engine {
public:
	Engine(World& wrld, std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_);
	~Engine();

	/* ends programm after finisheing the current tick */
	inline void quit() { running = false; }

	/* call run to start the rpogramm */
	virtual void run() final;

	/* specify what happenes once for initialisation */
	virtual void create() = 0;
	/* specify what happenes every update tick */
	virtual void update(float deltaTime) = 0;
	/* specify what happenes once for destruction */
	virtual void destroy() = 0;

					/*-- general statistics utility --*/
	/* returns time difference to last physics dispatch, O(1)*/
	inline float getDeltaTime() { return deltaTime; }
	/* returns deltatime or the lowest allowed sim time difference, O(1)*/
	inline float getDeltaTimeSafe() { return std::min(deltaTime, maxDeltaTime); }
	/* returns the number of past iterations , O(1)*/ 
	inline uint32_t getIteration() { return iteration; }
	/* returnes a string wtih formated performance info. The detail level changes how much information is shown, O(1) (os call) */
	std::string getPerfInfo(int detail);

					/*-- input utility --*/
	/* returns the status(KEYSTATUS) of a given key_(KEY), O(1) (mutex locking) */
	InputStatus getKeyStatus(KEY key);
	/* returns if a given key_ is pressed, O(1) (mutex locking) */
	bool keyPressed(KEY key);
	/* returns if a given key_ is released, O(1) (mutex locking) */
	bool keyReleased(KEY key);
	/* returns if a given key_ is repeating, O(1) (mutex locking) */
	bool keyRepeating(KEY key);
	/* returns mouse position in window relative coordinates, O(1) (mutex locking) */
	Vec2 getCursorPos();
	/* returns the keystatus of mouse buttons, O(1) (mutex locking) */
	InputStatus getButtonStatus(BUTTON but);
	/* returns true when a button is pressed, O(1) (mutex locking) */
	bool buttonPressed(BUTTON but);
	/* returns true when a button is NOT pressed, O(1) (mutex locking) */
	bool buttonReleased(BUTTON but);

					/*-- window utility --*/
	/* returns size of window in pixel of your desktop resolution, O(1)*/
	Vec2 getWindowSize();
	/* returns aspect ration width/height of the window, O(1)*/
	float getWindowAspectRatio();
	/* transformes world space coordinates into relative window space coordinates */
	Vec2 getPosWorldSpace(Vec2 windowSpacePos);
	Vec2 getPosWindowSpace(Vec2 worldSpacePos);

					/* graphics utility */
	/*  submit a Drawable to be rendered the next frame, O(1)  */
	void drawString(std::string str, std::string_view fontAtlas, Vec2 pos, Vec2 fontSize);
	void submitDrawable(Drawable && d);
	void submitDrawable(Drawable const& d);

private:
	void rendererUpdate(World& world);
public:
	World& world;
	EventHandler events;
	Camera camera;

	uint32_t freeDrawableID{ 0x80000000 };

	JobManager jobManager;

	// core systems
	BaseSystem baseSystem;
	MovementSystem movementSystem;
	CollisionSystem collisionSystem;
	//PhysicsSystem physicsSystem;
	PhysicsSystem2 physicsSystem2;

	PerfLogger perfLog;

private:
	// meta
	std::chrono::microseconds minimunLoopTime;
	bool running;
	uint32_t iteration;
	float maxDeltaTime;

	std::chrono::microseconds new_deltaTime;
	float deltaTime;

	// window
	std::shared_ptr<Window> window;
public:
	// render
	Renderer renderer;

	// UI
	UIManager ui;

};

inline void Engine::drawString(std::string str, std::string_view fontAtlas, Vec2 pos, Vec2 fontSize)
{
	pos += fontSize * 0.5f;
	int lineStride = 0;
	int lineBreakCount = 0;
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == '\n') {
			lineBreakCount++;
			lineStride = 0;
			continue;
		}
		auto drawID = ++freeDrawableID;
		auto d = Drawable(drawID, pos + Vec2(fontSize.x * lineStride, -fontSize.y * lineBreakCount), 1, fontSize, Vec4(0, 0, 0, 1), Form::Rectangle, RotaVec2(0.0f), DrawMode::PixelSpace, makeAsciiRef(world.texture.getId("ConsolasAtlas.png"), str[i]));
		auto background = Drawable(++freeDrawableID, pos + Vec2(fontSize.x * lineStride, -fontSize.y * lineBreakCount), 0.99, fontSize, Vec4(1, 1, 1, 0.9), Form::Rectangle, RotaVec2(0.0f), DrawMode::PixelSpace);
		submitDrawable(background);
		submitDrawable(d);
		lineStride++;
	}
}

__forceinline void Engine::submitDrawable(Drawable && d) {
	renderer.submit(d);
}

__forceinline void Engine::submitDrawable(Drawable const& d) {
	renderer.submit(d);
}

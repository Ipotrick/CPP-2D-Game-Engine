#pragma once

#include <chrono>
#include <iostream>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

// makro definitions:
#define DEBUG_STATIC_GRID

// ------------------

#include "robin_hood.h"

#include "Timing.h"
#include "BaseTypes.h"
#include "RenderTypes.h"
#include "QuadTree.h"
#include "Physics.h"
#include "input.h"
#include "Window.h"
#include "Camera.h"
#include "EventHandler.h"
#include "World.h"

#include "PhysicsWorker.h"
#include "Renderer.h"


class Engine
{
public:
	Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_);
	~Engine();

	/* ends programm after finisheing the current tick */
	inline void quit() { running = false; }

	/* call run to start the rpogramm */
	virtual void run() final;

	/* specify what happenes once for initialisation */
	virtual void create() = 0;
	/* specify what happenes every update tick */
	virtual void update(World& world, float deltaTime) = 0;
	/* specify what happenes once for destruction */
	virtual void destroy() = 0;
	

					/*-- general statistics utility --*/
	/* returns time difference to last physics dispatch, O(1)*/
	inline float getDeltaTime() { return deltaTime; }
	/* returns deltatime or the lowest allowed sim time difference, O(1)*/
	inline float getDeltaTimeSafe() { return std::min(deltaTime, maxDeltaTime); }
	/* returns physics + update + bufferSwapTime, O(1) */
	inline float getMainTime() { return mainTime; }
	/* returns time it took to process the last update task, O(1)*/
	inline float getUpdateTime() { return updateTime; }
	/* returns time it took to process the last physics task, O(1) */
	inline float getPhysicsTime() { return physicsTime; }
	/* returns time it took to render, O(1) */
	inline float getRenderTime() { return renderTime; }
	/* returns the number of past iterations , O(1)*/ 
	inline uint32_t getIteration() { return iteration; }
	/* returnes a string wtih formated performance info. The detail level changes how much information is shown, O(1) (os call) */
	std::string getPerfInfo(int detail);

					/*-- input utility --*/
	/* returns the status(KEYSTATUS) of a given key_(KEY), O(1) (mutex locking) */
	InputStatus getKeyStatus(KEY key_);
	/* returns if a given key_ is pressed, O(1) (mutex locking) */
	bool keyPressed(KEY key_);
	/* returns if a given key_ is released, O(1) (mutex locking) */
	bool keyReleased(KEY key_);
	/* returns if a given key_ is repeating, O(1) (mutex locking) */
	bool keyRepeating(KEY key_);
	/* returns mouse position in window relative coordinates, O(1) (mutex locking) */
	vec2 getCursorPos();
	/* returns the keystatus of mouse buttons, O(1) (mutex locking) */
	InputStatus getButtonStatus(BUTTON but_);
	/* returns true when a button is pressed, O(1) (mutex locking) */
	bool buttonPressed(BUTTON but_);
	/* returns true when a button is NOT pressed, O(1) (mutex locking) */
	bool buttonReleased(BUTTON but_);

					/*-- window utility --*/
	/* returns size of window in pixel of your desktop resolution, O(1)*/
	vec2 getWindowSize();
	/* returns aspect ration width/height of the window, O(1)*/
	float getWindowAspectRatio();
	/* transformes world space coordinates into relative window space coordinates */
	vec2 getPosWorldSpace(vec2 windowSpacePos);

					/* graphics utility */
	/* submit a Drawable to be drawn relative to the window, O(1) */
	void submitDrawableWindowSpace(Drawable d_);
	/* submit a Drawable to be drawn relative to the world, O(1) */
	void submitDrawableWorldSpace(Drawable d_);
				
					/* physics utility */
	/* returns a range (iterator to begin and end) of the collision list for the ent with the id, O(1) */
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisionInfos(uint32_t id_);
	/* CALL THIS WHENEVER YOU MOVE/ ADD/ REMOVE STATIC ENTITIES, O(1) */
	inline void staticsChanged() { 
		rebuildStaticData = true;
	}
	/* returns a Grid that with bools, if a cell is "true" there is a solid object, if it is "false" there is no solid object 
		the position of the cells can be calculated using the minPos and the cellSize member variables, O(1) */
	Grid<bool> const& getStaticGrid() { return staticGrid; }

public:
	World world;
	EventHandler events;
	Camera camera;

	std::chrono::microseconds minimunLoopTime;

private:
	void commitTimeMessurements();
	void physicsUpdate(World& world, float deltaTime);
	template<int N>
	void syncCompositPhysics();
	void updateStaticGrid(World& world);

private:
	// meta
	bool running;
	uint32_t iteration;
	float maxDeltaTime = 0.02f;
	bool rebuildStaticData{ true };

	// physics
	size_t oldWorldEntitiesCapacity;
	unsigned physicsThreadCount;
	std::vector<CollisionInfo> collInfos;
	robin_hood::unordered_map<uint32_t, std::vector<CollisionInfo>::iterator> collInfoBegins;
	robin_hood::unordered_map<uint32_t, std::vector<CollisionInfo>::iterator> collInfoEnds;
	std::vector<std::shared_ptr<PhysicsPerThreadData>> physicsPerThreadData;
	std::shared_ptr<PhysicsPoolData> physicsPoolData;
	std::shared_ptr<PhysicsSharedSyncData> sharedPhysicsSyncData;
	std::vector<std::thread> physicsThreads;
	uint32_t qtreeCapacity;

	// AI
	Grid<bool> staticGrid;

	// perf
	std::chrono::microseconds new_deltaTime;
	float deltaTime;
	std::chrono::microseconds new_mainTime;
	float mainTime;
	std::chrono::microseconds new_updateTime;
	float updateTime;
	std::chrono::microseconds new_physicsTime;
	float physicsTime;
	std::chrono::microseconds new_physicsPrepareTime;
	float physicsPrepareTime;
	std::chrono::microseconds new_physicsCollisionTime;
	float physicsCollisionTime;
	std::chrono::microseconds new_physicsExecuteTime;
	float physicsExecuteTime;
	std::chrono::microseconds new_staticGridBuildTime;
	float staticGridBuildTime;
	std::chrono::microseconds new_mainSyncTime;
	float mainSyncTime;
	std::chrono::microseconds new_mainWaitTime;
	float mainWaitTime;
	std::chrono::microseconds new_renderBufferPushTime;
	float renderBufferPushTime;
	std::chrono::microseconds new_renderTime;
	float renderTime;
	std::chrono::microseconds new_renderSyncTime;
	float renderSyncTime;

	// window
	std::shared_ptr<Window> window;
	std::shared_ptr<RendererSharedData> sharedRenderData;
	std::thread renderThread;
	RenderBuffer renderBufferA;

	// render
	std::vector<Drawable> windowSpaceDrawables;
	std::vector<Drawable> worldSpaceDrawables;
};

inline void Engine::submitDrawableWindowSpace(Drawable d_) {
	windowSpaceDrawables.emplace_back(d_);
}

inline void Engine::submitDrawableWorldSpace(Drawable d_) {
	worldSpaceDrawables.emplace_back(d_);
}
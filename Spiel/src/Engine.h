#pragma once

#include <chrono>
#include <iostream>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "QuadTree.h"
#include "input.h"
#include "Timing.h"
#include "Window.h"
#include "Camera.h"
#include "Physics.h"
#include "Entity.h"
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
	/* returnes a string wtih formated performance info. The detail level changes how much information is shown */
	std::string getPerfInfo(int detail);

					/*-- input utility --*/
	/* returns the status(KEYSTATUS) of a given key_(KEY), O(1) (mutex locking) */
	KEYSTATUS getKeyStatus(KEY key_);
	/* returns if a given key_ is pressed, O(1) (mutex locking) */
	bool keyPressed(KEY key_);
	/* returns if a given key_ is released, O(1) (mutex locking) */
	bool keyReleased(KEY key_);
	/* returns if a given key_ is repeating, O(1) (mutex locking) */
	bool keyRepeating(KEY key_);

					/*-- window utility --*/
	/* returns size of window in pixel of your desktop resolution, O(1)*/
	vec2 getWindowSize();
	/* returns aspect ration width/height of the window, O(1)*/
	float getWindowAspectRatio();

					/* graphics utility */
	/* submit a Drawable to be drawn relative to the window, O(1) */
	void submitDrawableWindowSpace(Drawable d_);
	/* submit a Drawable to be drawn relative to the world, O(1) */
	void submitDrawableWorldSpace(Drawable d_);
				
					/* physics utility */
	/* returns a range (iterator to begin and end) of the collision list for the ent, O(log2(n)) */
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisionInfos(Entity const& ent_);
	/* returns a range (iterator to begin and end) of the collision list for the ent with the id, O(log2(n)) */
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisionInfos(uint32_t id_);

public:
	World world;
	Camera camera;

	std::chrono::microseconds minimunLoopTime;

private:
	void commitTimeMessurements();
	void physicsUpdate(World& world, float deltaTime);

private:
	bool running;
	uint32_t iteration;
	float maxDeltaTime = 0.010;

	int physicsThreadCount;
	std::vector<CollisionInfo> collisionInfos;
	std::vector<std::shared_ptr<PhysicsSharedData>> sharedPhysicsData;
	std::shared_ptr<PhysicsSyncData> sharedPhysicsSyncData;
	std::vector<std::thread> physicsThreads;
	uint32_t qtreeCapacity;

	std::shared_ptr<Window> window;

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
	
	std::shared_ptr<RendererSharedData> sharedRenderData;
	std::thread renderThread;
	RenderBuffer renderBufferA;

	std::vector<Drawable> windowSpaceDrawables;
	std::vector<Drawable> worldSpaceDrawables;
};

inline void Engine::submitDrawableWindowSpace(Drawable d_) {
	windowSpaceDrawables.emplace_back(d_);
}

inline void Engine::submitDrawableWorldSpace(Drawable d_) {
	worldSpaceDrawables.emplace_back(d_);
}

inline std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> Engine::getCollisionInfos(Entity const& ent_) {
	return getCollisionInfos(ent_.getId());
}
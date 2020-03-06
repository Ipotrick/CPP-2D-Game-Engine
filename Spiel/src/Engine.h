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

#include "Renderer.h"

class Engine
{
public:
	Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_);
	~Engine();

	virtual void run() final;

	virtual void create() = 0;
	virtual void update(World& world, float deltaTime) = 0;
	virtual void destroy() = 0;

	virtual void physicsUpdate(World& world, float deltaTime) final;

					/*-- general statistics utility --*/
	/* returns time difference to last physics dispatch*/
	inline float getDeltaTime() { return deltaTime; }
	/* returns deltatime or the lowest allowed sim time difference */
	inline float getDeltaTimeSafe() { return std::min(deltaTime, maxDeltaTime); }
	/* returns physics + update + bufferSwapTime */
	inline float getMainTime() { return mainTime; }
	/* returns time it took to process the last update task*/
	inline float getUpdateTime() { return updateTime; }
	/* returns time it took to process the last physics task */
	inline float getPhysicsTime() { return physicsTime; }
	/* returns time it took to render */
	inline float getRenderTime() { return renderTime; }
	/* returns the number of past iterations */ 
	inline uint32_t getIteration() { return iteration; }
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

					/* graphic utility */
	void submitDrawableWindowspace(Drawable d_);
	void submitDrawableWorldSpace(Drawable d_);
				
					/* physics utility */
	/* returns a range (iterator to begin and end) of the collision list for the ent */
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisionInfos(Entity const& ent_);
	/* returns a range (iterator to begin and end) of the collision list for the ent with the id */
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisionInfos(uint32_t id_);

public:
	World world;
	Camera camera;

	std::chrono::microseconds minimunLoopTime;

private:
	void commitTimeMessurements();

private:
	bool running;
	uint32_t iteration;
	float maxDeltaTime = 0.016;

	int physicsThreadCount;

	std::vector<CollisionInfo> collisionInfos;

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

inline void Engine::submitDrawableWindowspace(Drawable d_) {
	windowSpaceDrawables.emplace_back(d_);
}

inline void Engine::submitDrawableWorldSpace(Drawable d_) {
	worldSpaceDrawables.emplace_back(d_);
}

inline std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> Engine::getCollisionInfos(Entity const& ent_) {
	return getCollisionInfos(ent_.getId());
}
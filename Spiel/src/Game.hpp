#pragma once

#include <iostream>

#include "Engine.hpp"
// Core Systems
#include "CollisionSystem.hpp"
#include "PhysicsSystem2.hpp"

struct CursorManipData {
	CursorManipData() : locked{ false }, ballSpawnLap{ 0.01 }, wallSpawnLap{ 0.1 } {}

	bool locked;
	Vec2 oldPos{0,0};
	EntityHandle lockedID;
	Vec2 relativePos;
	LapTimer ballSpawnLap;
	LapTimer wallSpawnLap;
};

class Game : public Engine {
public:
	Game();

	void create() override;

	void update(float dTime) override;

	void destroy() override;

	void gameplayUpdate(float deltaTime);

	void cursorManipFunc();

	inline static CursorManipData cursorData;
	inline static CollisionSystem collisionSystem{ world, jobManager, perfLog };
	inline static PhysicsSystem2 physicsSystem2{ jobManager, perfLog };

	// TEMP TODO REMOVE
	LapTimer spawnerLapTimer{0.0001f};

	bool bLoading{ false };
	Tag loadingWorkerTag{ Tag() };
	World* loadedWorld{ nullptr };
};
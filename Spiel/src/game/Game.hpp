#pragma once

#include <iostream>

#include "collision/CollisionSystem.hpp"

#include "../engine/EngineCore.hpp"
#include "World.hpp"
using Coll = Collider;
using Move = Movement;
// Core Systems
#include "physics/PhysicsSystem2.hpp"

struct CursorManipData {
	CursorManipData() : locked{ false }, ballSpawnLap{ 0.01 }, wallSpawnLap{ 0.1 } {}

	bool locked;
	Vec2 oldPos{0,0};
	EntityHandle lockedID;
	Vec2 relativePos;
	LapTimer ballSpawnLap;
	LapTimer wallSpawnLap;
};

class Game : public EngineCore {
public:
	Game();

	void create() override;

	void update(float dTime) override;

	void destroy() override;

	void gameplayUpdate(float deltaTime);

	void cursorManipFunc();

	inline static World world;

	inline static CursorManipData cursorData;
	inline static CollisionSystem collisionSystem{ world.submodule<COLLISION_SECM_COMPONENTS>() };
	inline static PhysicsSystem2 physicsSystem2{ jobManager };

	// TEMP TODO REMOVE
	LapTimer spawnerLapTimer{0.0001f};

	bool bLoading{ false };
	Tag loadingWorkerTag{ Tag() };
	World* loadedWorld{ nullptr };
};
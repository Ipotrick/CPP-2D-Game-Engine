#pragma once

#include <iostream>

#include "../engine/collision/CollisionSystem.hpp"
#include "../engine/rendering/DefaultRenderer.hpp"

#include "../engine/EngineCore.hpp"
#include "World.hpp"
using Coll = Collider;
using Move = Movement;
// Core Systems
#include "../engine/physics/PhysicsSystem2.hpp"

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

	World world;

	CursorManipData cursorData;
	CollisionSystem collisionSystem{ world.submodule<COLLISION_SECM_COMPONENTS>() };
	PhysicsSystem2 physicsSystem2;

	// TEMP TODO REMOVE
	LapTimer spawnerLapTimer{0.0001f};

	bool bLoading{ false };
	JobSystem::Tag loadingWorkerTag{ JobSystem::Tag() };
	World loadedWorld;

	DefaultRenderer renderer;
};
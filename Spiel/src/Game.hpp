#pragma once

#include <iostream>

#include "Engine.hpp"
#include "GameWorld.hpp"
#include "GameComponents.hpp"
#include "BasicScripts.hpp"
#include "ParticleScript.hpp"
#include "DummyScript.hpp"
#include "SuckerScript.hpp"

struct CursorManipData {
	CursorManipData() : locked{ false }, ballSpawnLap{ 0.01 }, wallSpawnLap{ 0.1 } {}

	bool locked;
	Vec2 oldCursorPos;
	uint32_t lockedID;
	Vec2 lockedIDDist;
	LapTimer<> ballSpawnLap;
	LapTimer<> wallSpawnLap;
};

class Game : public Engine {
public:
	Game();

	void create() override;

	void update(World& world, float dTime) override;

	void gameplayUpdate(World& world, float deltaTime);

	void destroy() override {}

public:
	GameWorld world;
	PlayerScript	playerScript;
	HealthScript	healthScript;
	AgeScript		ageScript;
	BulletScript	bulletScript;
	ParticleScript	particleScript;
	DummyScript		dummyScript;
	SuckerScript	suckerScript;

	EntityId cursorID;

	CursorManipData cursorManipData;
	void cursorManipFunc();

	static bool testEventReaction(std::string_view, uint32_t);

	// TEMP TODO REMOVE
	LapTimer<> spawnerLapTimer{0.001f};
};

inline bool Game::testEventReaction(std::string_view name, uint32_t id) {
	std::cout << "player got hit" << std::endl;
	return false;
}
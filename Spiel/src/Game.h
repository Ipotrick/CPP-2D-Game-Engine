#pragma once

#include <iostream>

#include "Engine.h"
#include "GameWorld.h"
#include "GameComponents.h"
#include "BasicScripts.h"
#include "ParticleScript.h"
#include "DummyScript.h"

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

	void destroy() override {}

public:
	GameWorld world;
	PlayerScript	playerScript;
	HealthScript	healthScript;
	AgeScript		ageScript;
	BulletScript	bulletScript;
	ParticleScript	particleScript;
	DummyScript		dummyScript;

	entity_id cursorID;

	CursorManipData cursorManipData;
	void cursorManipFunc();

	static bool testEventReaction(std::string_view, uint32_t);
};

inline bool Game::testEventReaction(std::string_view name, uint32_t id) {
	std::cout << "player got hit" << std::endl;
	return false;
}
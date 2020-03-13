#pragma once

#include <iostream>

#include "Engine.h"
#include "GameComponents.h"
#include "ScriptInclude.h"

class Game : public Engine {
public:
	Game();

	void create() override;

	void update(World& world, float dTime) override;

	void destroy() override {}

public:
	PlayerScript playerScript;
	HealthScript healthScript;
	AgeScript    ageScript;
	BulletScript bulletScript;
	uint32_t cursorID;
	uint32_t attractorID;
	uint32_t pusherID;
};
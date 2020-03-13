#pragma once

#include <iostream>

#include "Engine.h"
#include "GameComponents.h"
#include "ScriptInclude.h"

struct CursorManipData {
	CursorManipData() : locked{ false } {}

	bool locked;
	vec2 oldCursorPos;
	uint32_t lockedID;
	vec2 lockedIDDist;
};

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
	TriggerScript triggerScript;
	OwnerScript ownerScript;
	SlaveScript slaveScript;
	uint32_t cursorID;
	
	uint32_t attractorID;
	uint32_t pusherID;

	CursorManipData cursorManipData;
	void cursorManipFunc();
};
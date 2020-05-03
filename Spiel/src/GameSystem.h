#pragma once

#include "Engine.h"

class GameSystem {
public:
	GameSystem(Engine& engine) : engine{ engine } {}
	void execute(float deltaTime);
private:
	Engine& engine;
};
#pragma once

#include "Engine.hpp"

class GameSystem {
public:
	GameSystem(Engine& engine) : engine{ engine } {}
	void execute(float deltaTime);
private:
	Engine& engine;
};
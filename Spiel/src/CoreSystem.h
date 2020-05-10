#pragma once

#include "World.h"

class CoreSystem {
public:
	CoreSystem(World& world) : world{ world } {}
	void execute(float deltaTime);
protected:
	World& world;
};
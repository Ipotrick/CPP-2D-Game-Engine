#pragma once

#include "World.hpp"

class CoreSystem {
public:
	CoreSystem(World& world) : world{ world } {}
	void execute(World& world, float deltaTime);
protected:
	World& world;
};
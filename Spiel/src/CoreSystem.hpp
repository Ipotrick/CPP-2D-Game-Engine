#pragma once

#include "World.hpp"

class CoreSystem {
public:
	virtual void execute(World& world, float deltaTime) = 0;
protected:
};
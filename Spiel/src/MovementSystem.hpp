#pragma once

#include "CoreSystem.hpp"
#include "JobManager.hpp"
#include "Perf.hpp"
#include "PhysicsTypes.hpp"

class MovementSystem : public CoreSystem {
public:
	void execute(World& world, float deltaTime);
	void end();
};
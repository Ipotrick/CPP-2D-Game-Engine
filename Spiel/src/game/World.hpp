#pragma once

#include "EngineConfig.hpp"
#include "../engine/EntityComponentManager.hpp"
#include "collision/CoreSystemUniforms.hpp"

class World : public EntityComponentManager {
public:
	using EntityComponentManager::EntityComponentManager;

	// Core System Uniform Data
	PhysicsUniforms physics;
private:
};
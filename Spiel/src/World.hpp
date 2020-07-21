#pragma once

#include "EntityComponentManager.hpp"

#include "CoreSystemUniforms.hpp"

class World : public EntityComponentManager {
public:
	using EntityComponentManager::EntityComponentManager;

	void loadMap(std::string);
public:
	// Core System Uniform Data
	PhysicsUniforms physics;
private:
private:
};
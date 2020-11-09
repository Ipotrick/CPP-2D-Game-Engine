#pragma once
#include "EntityComponentManager.hpp"

#include "CoreSystemUniforms.hpp"

class World : public EntityComponentManager {
public:
	using EntityComponentManager::EntityComponentManager;

	virtual void loadMap(const std::string& filename);
	virtual void saveMap(const std::string& filename);

	// Core System Uniform Data
	PhysicsUniforms physics;
private:
};
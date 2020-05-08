#pragma once

#include "EntityComponentManager.h"

#include "CoreSystemUniforms.h"

class World : public EntityComponentManager {
public:
	using EntityComponentManager::EntityComponentManager;
	// updates internals:
	// executes defragmentation
	// executes despawns
	// executes delayedSpawns
	void tick() { 
		update();
	}

	void loadMap(std::string);
public:
	// Core System Uniform Data
	PhysicsUniforms physics;
private:
private:
};
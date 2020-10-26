#include "EntityComponentManager.hpp"

void EntityComponentManager::update()
{
	executeDelayedSpawns();
	deregisterDestroyedEntities();
	executeDestroys();
}
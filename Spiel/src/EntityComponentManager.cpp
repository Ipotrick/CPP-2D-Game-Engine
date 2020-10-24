#include "EntityComponentManager.hpp"

void EntityComponentManager::update()
{
	executeDelayedSpawns();
	deregisterDestroyedEntities();
	executeDestroys();
	//std::sort(freeDynamicIdQueue.begin(), freeDynamicIdQueue.end());
	//std::sort(freeStaticIdQueue.begin(), freeStaticIdQueue.end());
	//std::sort(freeIndexQueue.begin(), freeIndexQueue.end());
}
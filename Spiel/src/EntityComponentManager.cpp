#include "EntityComponentManager.hpp"

EntityHandle EntityComponentManager::create(UUID uuid)
{
	EntityHandle ent = EntityManager::create(uuid);
	updateMaxEntityToComponentSotrages();
	return ent;
}

void EntityComponentManager::update()
{
	executeDelayedSpawns();
	deregisterDestroyedEntities();
	executeDestroys();
}

void EntityComponentManager::updateMaxEntityToComponentSotrages()
{
	util::tuple_for_each(componentStorageTuple,
		[&](auto& componentStorage) {
			componentStorage.updateMaxEntNum(entitySlots.size());
		}
	);
}

void EntityComponentManager::deregisterDestroyedEntities()
{
	util::tuple_for_each(componentStorageTuple,
		[&](auto& componentStorage) {
			for (EntityHandleIndex entity : destroyQueue) {
				if (componentStorage.contains(entity)) {
					componentStorage.remove(entity);
				}
			}
		}
	);
}

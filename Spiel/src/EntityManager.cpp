#include "EntityManager.hpp"

EntityHandle EntityManager::create()
{
	EntityHandle ent;
	if (!freeIndexQueue.empty()) {
		ent.index = freeIndexQueue.front();
		freeIndexQueue.pop_front();
		auto& slot = entitySlots[ent.index];
		slot.setHoldsEntity(true);
		slot.setSpawned(false);
		ent.version = ++slot.version;
	}
	else {
		entitySlots.emplace_back(EntitySlot(true));
		ent.index = static_cast<EntityHandleIndex>(entitySlots.size() - 1);
		ent.version = ++entitySlots[ent.index].version;
	}
	return ent;
}

void EntityManager::destroy(EntityHandle entity)
{
	if (isHandleValid(entity)) {
		destroyQueue.push_back(entity.index);
	}
}

void EntityManager::spawnLater(EntityHandle entity)
{
	spawnLaterQueue.emplace_back(entity);
}

size_t const EntityManager::size()
{
	return entitySlots.size() - (freeIndexQueue.size() + 1);
}

size_t const EntityManager::maxEntityIndex()
{
	return entitySlots.size();
}

EntityHandleIndex EntityManager::findBiggestValidEntityIndex()
{
	for (int i = entitySlots.size() - 1; i > 0; i--) {
		if (entitySlots[i].holdsEntity()) return i;
	}
	return 0;
}

void EntityManager::executeDestroys()
{
	for (EntityHandleIndex entSlotIndex : destroyQueue) {
		entitySlots[entSlotIndex].setHoldsEntity(false);
		entitySlots[entSlotIndex].setSpawned(false);
		freeIndexQueue.push_back(entSlotIndex);
	}
	destroyQueue.clear();
}

void EntityManager::executeDelayedSpawns()
{
	for (auto ent : spawnLaterQueue) {
		if (isHandleValid(ent))
			spawn(ent);
	}
	spawnLaterQueue.clear();
}
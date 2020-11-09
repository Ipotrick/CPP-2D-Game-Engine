#include "EntityManager.hpp"

EntityHandle EntityManager::create(UUID uuid)
{
	EntityHandle ent;
	if (!freeIndexQueue.empty()) {
		ent.index = freeIndexQueue.front();
		freeIndexQueue.pop_front();
		auto& slot = entitySlots[ent.index];
		slot.valid = true;
		slot.spawned = false;
		ent.version = ++slot.version;
	}
	else {
		entitySlots.emplace_back(EntitySlot(true));
		ent.index = static_cast<EntityHandleIndex>(entitySlots.size() - 1);
		ent.version = ++entitySlots[ent.index].version;
	}
	if (uuid.isValid()) {
		assertEntityManager(!uuidToEntityIndex.contains(uuid));
		entitySlots[ent.index].uuid = uuid;
		uuidToEntityIndex[uuid] = ent.index;
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
	assertEntityManager(isHandleValid(entity));
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
		if (entitySlots[i].valid) return i;
	}
	return 0;
}

void EntityManager::executeDestroys()
{
	for (EntityHandleIndex entSlotIndex : destroyQueue) {
		auto& entityData = entitySlots[entSlotIndex];
		entityData.valid = false;
		entityData.spawned = false;
		if (entityData.uuid.isValid()) {
			uuidToEntityIndex.erase(entityData.uuid);
		}
		entityData.uuid.invalidate();
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
#include "EntityManager.hpp"

Entity EntityManager::create()
{
	Entity ent;
	if (!freeIndexQueue.empty()) {
		Entity index = freeIndexQueue.front();
		freeIndexQueue.pop_front();
		entityStatusVec[index].setValid(true);
		entityStatusVec[index].setSpawned(false);
		ent = index;
	}
	else {
		entityStatusVec.emplace_back(true);
		ent = static_cast<Entity>(entityStatusVec.size() - 1);
	}
	makeDynamicId(ent);
	return ent;
}

EntityId EntityManager::makeDynamicId(Entity entity)
{
	assert(exists(entity));
	if (indexToIdTable.size() != entityStatusVec.size()) indexToIdTable.resize(entityStatusVec.size(), INVALID_ID);
	if (hasId(entity)) /* does the handle allready have an id? */ {
		throw new std::exception();
	}
	else {
		// generate id for entity
		if (!freeDynamicIdQueue.empty()) {
			// reuse existing index of id vector
			auto id = freeDynamicIdQueue.front();
			freeDynamicIdQueue.pop_front();

			// emplace new dynamic id:
			idToIndexTable[id] = entity;
			idToVersionTable[id] += 1;	// for every reuse the version gets an increase
			indexToIdTable[entity] = id;

			return EntityId(id, idToVersionTable[id]);
		}
		else {
			// expand id vectors

			// push back memory space for static id:
			idToIndexTable.push_back(INVALID_ENTITY);
			idToVersionTable.emplace_back(0);
			freeStaticIdQueue.push_back(idToIndexTable.size() - 1);

			// emplate new dynamic id:
			idToIndexTable.push_back(entity);
			idToVersionTable.emplace_back(0);
			entity_id_t id = idToIndexTable.size() - 1;
			indexToIdTable[entity] = id;

			return EntityId(id, 0);
		}
	}
}

EntityId EntityManager::makeStaticId(Entity entity)
{
	assert(exists(entity));
	if (indexToIdTable.size() != entityStatusVec.size()) indexToIdTable.resize(entityStatusVec.size(), INVALID_ID);
	if (hasId(entity)) /* does the handle allready have an id? */ {
		throw new std::exception();
	}
	else {
		// generate id for entity
		if (!freeStaticIdQueue.empty()) {
			// reuse existing index of id vector
			auto id = freeStaticIdQueue.front();
			freeStaticIdQueue.pop_front();

			// emplace new static id:
			idToIndexTable[id] = entity;
			idToVersionTable[id] += 1;	// for every reuse the version gets an increase
			indexToIdTable[entity] = id;

			return EntityId(id, idToVersionTable[id]);
		}
		else {
			// expand id vector

			// emplace new static id:
			idToIndexTable.push_back(entity);
			idToVersionTable.emplace_back(0);
			entity_id_t id = idToIndexTable.size() - 1;
			indexToIdTable[entity] = id;

			// push back memory space for dynamic id:
			idToIndexTable.push_back(INVALID_ENTITY);
			idToVersionTable.emplace_back(0);
			freeDynamicIdQueue.push_back(idToIndexTable.size() - 1);

			return EntityId(id, 0);
		}
	}
}

void EntityManager::destroy(Entity index)
{
	if (index < entityStatusVec.size()) {
		assert(entityStatusVec[index].isValid());
		destroyQueue.push_back(index);
	}
}

void EntityManager::destroy(EntityId id)
{
	destroy(idToIndexTable[id.identifier]);
}

void EntityManager::spawnLater(Entity entity)
{
	spawnLaterQueue.emplace_back(entity);
}

void EntityManager::spawnLater(EntityId id)
{
	spawnLater(idToIndexTable[id.identifier]);
}

size_t const EntityManager::size()
{
	return entityStatusVec.size() - (freeIndexQueue.size() + 1);
}

size_t const EntityManager::maxEntityIndex()
{
	return entityStatusVec.size();
}

Entity EntityManager::findBiggestValidEntityIndex()
{
	for (int i = entityStatusVec.size() - 1; i > 0; i--) {
		if (entityStatusVec[i].isValid()) return i;
	}
	return 0;
}

void EntityManager::executeDestroys()
{

	for (Entity index : destroyQueue) {
		if (hasId(index)) {	// if the index has no id, the entity allready got destroyed
			// reset id references:
			auto id = indexToIdTable[index];
			if (id & 1) {
				freeDynamicIdQueue.push_back(id);
			}
			else {
				freeStaticIdQueue.push_back(id);
			}
			indexToIdTable[index] = INVALID_ID;
			idToIndexTable[id] = INVALID_ENTITY;
			// reset status of handle:
			entityStatusVec[index].setValid(false);
			entityStatusVec[index].setSpawned(false);
			freeIndexQueue.push_back(index);
		}
	}
	destroyQueue.clear();
}

void EntityManager::executeDelayedSpawns()
{
	for (auto ent : spawnLaterQueue) {
		spawn(ent);
	}
	spawnLaterQueue.clear();
}

void EntityManager::shrink()
{
	// !! handles must be sorted !!
	auto lastEl = findBiggestValidEntityIndex();
	entityStatusVec.resize(lastEl + 1LL, EntityStatus(false));
	std::vector<Entity> handleVec;

	for (int i = freeIndexQueue.size() - 1; i >= 0; i--) {
		if (freeIndexQueue.at(i) < entityStatusVec.size()) break;
		freeIndexQueue.pop_back();
	}
}

float EntityManager::fragmentation()
{
	return (float)freeIndexQueue.size() / (float)maxEntityIndex();
}
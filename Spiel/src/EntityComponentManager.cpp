#include "EntityComponentManager.hpp"

Entity EntityComponentManager::index_create() {
	if (!freeIndexQueue.empty()) {
		Entity index = freeIndexQueue.front();
		freeIndexQueue.pop_front();
		entityStorageInfo[index].setValid(true);
		entityStorageInfo[index].setDestroyMark(false);
		entityStorageInfo[index].setSpawned(false);
		latestIndex = index;
	}
	else {
		entityStorageInfo.emplace_back( true );
		latestIndex = static_cast<Entity>(entityStorageInfo.size() - 1);

		for_each(componentStorageTuple, [&](auto& componentStorage) {
			componentStorage.updateMaxEntNum(entityStorageInfo.size());
			});
	}
	makeDynamicId(latestIndex);	// TODO replace with optimised version
	return latestIndex;
}

EntityId EntityComponentManager::idCreate()
{
	auto index = index_create();
	return getId(index);
}

void EntityComponentManager::destroy(Entity index) {
	if (index < entityStorageInfo.size() && !entityStorageInfo[index].isDestroyMarked()) {
		assert(entityStorageInfo[index].isValid());
		entityStorageInfo[index].setDestroyMark(true);
		despawnList.push_back(index);
	}
}

void EntityComponentManager::destroy(EntityId id)
{
	destroy(idToIndex[id.id]);
}

void EntityComponentManager::spawnLater(Entity entity)
{
	spawnLaterList.emplace_back(entity);
}

void EntityComponentManager::spawnLater(EntityId id)
{
	spawnLater(idToIndex[id.id]);
}

EntityId EntityComponentManager::makeDynamicId(Entity entity)
{
	assert(exists(entity));
	if (indexToId.size() != entityStorageInfo.size()) indexToId.resize(entityStorageInfo.size(), 0 );
	if (indexToId[entity] != 0) /* does the handle allready have an id? */ {
		throw new std::exception();
	}
	else {
		// generate id for entity
		if (!freeDynamicIdQueue.empty()) {
			// reuse existing index of id vector
			auto id = freeDynamicIdQueue.front();
			freeDynamicIdQueue.pop_front();

			// emplace new dynamic id:
			idToIndex[id] = entity;
			idToVersion[id] += 1;	// for every reuse the version gets an increase
			indexToId[entity] = id;

			return EntityId(id, idToVersion[id]);
		}
		else {
			// expand id vectors

			// push back memory space for static id:
			idToIndex.push_back(0);
			idToVersion.emplace_back(0);
			freeStaticIdQueue.push_back(idToIndex.size() - 1);

			// emplate new dynamic id:
			idToIndex.push_back(entity);
			idToVersion.emplace_back(0);
			entity_id_t id = idToIndex.size() - 1;
			indexToId[entity] = id;

			return EntityId(id, 0);
		}
	}
}

EntityId EntityComponentManager::makeStaticId(Entity entity)
{
	assert(exists(entity));
	if (indexToId.size() != entityStorageInfo.size()) indexToId.resize(entityStorageInfo.size(), 0);
	if (indexToId[entity] != 0) /* does the handle allready have an id? */ {
		throw new std::exception();
	}
	else {
		// generate id for entity
		if (!freeStaticIdQueue.empty()) {
			// reuse existing index of id vector
			auto id = freeStaticIdQueue.front();
			freeStaticIdQueue.pop_front();

			// emplace new static id:
			idToIndex[id] = entity;
			idToVersion[id] += 1;	// for every reuse the version gets an increase
			indexToId[entity] = id;

			return EntityId(id, idToVersion[id]);
		}
		else {
			// expand id vector

			// emplace new static id:
			idToIndex.push_back(entity);
			idToVersion.emplace_back(0);
			entity_id_t id = idToIndex.size() - 1;
			indexToId[entity] = id;

			// push back memory space for dynamic id:
			idToIndex.push_back(0);
			idToVersion.emplace_back(0);
			freeDynamicIdQueue.push_back(idToIndex.size() - 1);

			return EntityId(id, 0);
		}
	}
}

void EntityComponentManager::executeDestroys() {
	for (Entity index : despawnList) {
		if (hasComp<Collider>(index) && !hasComp<Movement>(index)) { staticEntitiesChanged = true; }
		// reset id references:
		if (!hasId(index)) {
			throw new std::exception();
		}
		auto id = indexToId[index];
		if (id & 1) {
			freeStaticIdQueue.push_back(id);
		}
		else {
			freeDynamicIdQueue.push_back(id);
		}
		indexToId[index] = 0;
		idToIndex[id] = 0;
		// reset status of handle:
		entityStorageInfo[index].setValid(false);
		entityStorageInfo[index].setDestroyMark(false);
		entityStorageInfo[index].setSpawned(false);
		freeIndexQueue.push_back(index);
	}
	despawnList.clear();
}

void EntityComponentManager::flushLaterActions()
{
	despawnList.reserve(entityStorageInfo.size());	// make sure the iterators stay valid
	executeDelayedSpawns();
	deregisterDestroyedEntities();
	executeDestroys();
	std::sort(freeDynamicIdQueue.begin(), freeDynamicIdQueue.end());
	std::sort(freeIndexQueue.begin(), freeIndexQueue.end());
}

void EntityComponentManager::moveEntity(Entity start, Entity goal)
{
	assert(entityStorageInfo.size() > goal && entityStorageInfo[goal].isValid() == false);
	assert(entityStorageInfo.size() > start && entityStorageInfo[start].isValid() == true); 
	if (hasId(start)) {
		idToIndex[indexToId[start]] = goal;
		indexToId[goal] = indexToId[start];
		indexToId[start] = 0;
	}
	for_each(componentStorageTuple, [&](auto& componentStorage) {
		if (componentStorage.contains(start)) {
			componentStorage.insert(goal, componentStorage.get(start));
			componentStorage.remove(start);
			assert(componentStorage.contains(goal));
			assert(!componentStorage.contains(start));
		}
		});
	entityStorageInfo[goal].setValid(true);
	entityStorageInfo[goal].setSpawned(entityStorageInfo[start].isSpawned());
	entityStorageInfo[start].setValid(false);
	entityStorageInfo[start].setDestroyMark(false);
	entityStorageInfo[start].setSpawned(false);
}

Entity EntityComponentManager::findBiggestValidHandle()
{
	for (int i = entityStorageInfo.size() - 1; i > 0; i--) {
		if (entityStorageInfo[i].isValid()) return i;
	}
	return 0;
}

void EntityComponentManager::shrink() {
	// !! handles must be sorted !!
	auto lastEl = findBiggestValidHandle();
	entityStorageInfo.resize(lastEl + 1LL, EntityStatus(false));
	std::vector<Entity> handleVec;

	for (int i = freeIndexQueue.size() - 1; i >= 0; i--) {
		if (freeIndexQueue.at(i) < entityStorageInfo.size()) break;
		freeIndexQueue.pop_back();
	}
}

void EntityComponentManager::defragment(DefragMode const mode)
{
	int maxDefragEntCount;
	float minFragmentation(1.0f);
	switch (mode) {
	case DefragMode::FAST:
		maxDefragEntCount = 10;
		minFragmentation = 0.001;
		break;
	case DefragMode::NONE:
		maxDefragEntCount = 0;
		minFragmentation = 1.01f;
		break;
	case DefragMode::LAZY:
		maxDefragEntCount = 200;
		minFragmentation = 0.5f;
		break;
	case DefragMode::MODERATE:
		maxDefragEntCount = 200;
		minFragmentation = 0.33333f;
		break;
	case DefragMode::EAGER:
		maxDefragEntCount = 100;
		minFragmentation = 0.1f;
		break;
	case DefragMode::AGRESSIVE:
		maxDefragEntCount = 50;
		minFragmentation = 0.01f;
		break;
	case DefragMode::COMPLETE:
		maxDefragEntCount = maxEntityIndex();
		minFragmentation = 0.00001f;
		break;
	}

	if (fragmentation() > minFragmentation) {
		shrink();
		
		for (int defragCount = 0; defragCount < maxDefragEntCount; defragCount++) {
			if (freeIndexQueue.empty()) break;
			auto biggesthandle = findBiggestValidHandle();
			moveEntity(biggesthandle, freeIndexQueue.front());
			freeIndexQueue.pop_front();
			shrink();
		}
	}
}

void EntityComponentManager::deregisterDestroyedEntities() {
	for (Entity entity : despawnList) {
		for_each(componentStorageTuple, [&](auto& componentStorage) {
			componentStorage.remove(entity);
			});
	}
}

void EntityComponentManager::executeDelayedSpawns()
{
	for (auto ent : spawnLaterList) {
		spawn(ent);
	}
	spawnLaterList.clear();
}

size_t const EntityComponentManager::entityCount() {
	return entityStorageInfo.size() - (freeIndexQueue.size() + 1);
}

size_t const EntityComponentManager::maxEntityIndex() {
	return entityStorageInfo.size();
}

void EntityComponentManager::setStaticsChanged(bool boolean)
{
	staticEntitiesChanged = boolean;
}

bool EntityComponentManager::didStaticsChange()
{
	return staticEntitiesChanged;
}

float EntityComponentManager::fragmentation()
{
	return (float)freeIndexQueue.size() / (float)maxEntityIndex();
}

float randomFloatd(float MaxAbsVal) {
	float randomNum = rand() % 1'000 / 1'000.0f;
	randomNum -= 0.5f;
	randomNum *= MaxAbsVal;
	return randomNum;
}
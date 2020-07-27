#include "EntityComponentManager.hpp"

entity_index_type EntityComponentManager::index_create() {
	if (!freeIndexQueue.empty()) {
		entity_index_type index = freeIndexQueue.front();
		freeIndexQueue.pop_front();
		entityStorageInfo[index].setValid(true);
		entityStorageInfo[index].setDestroyMark(false);
		entityStorageInfo[index].setSpawned(false);
		latestIndex = index;
	}
	else {
		entityStorageInfo.emplace_back( true );
		latestIndex = static_cast<entity_index_type>(entityStorageInfo.size() - 1);

		for_each(componentStorageTuple, [&](auto& componentStorage) {
			componentStorage.updateMaxEntNum(entityStorageInfo.size());
			});
	}
	identify(latestIndex);	// TODO replace with optimised version
	return latestIndex;
}

entity_id EntityComponentManager::create()
{
	auto index = index_create();
	return identify(index);
}

void EntityComponentManager::link(entity_index_type slave, entity_index_type master, Vec2 relativePos, float relativeRota)
{
	if (hasntComp<Parent>(master)) addComp<Parent>(master);
	auto& parent = getComp<Parent>(master);

	parent.children.push_back(identify(slave));

	if (hasntComp<BaseChild>(slave)) addComp<BaseChild>(slave);
	auto& baseChild = getComp<BaseChild>(slave);

	baseChild.relativePos = relativePos;
	baseChild.relativeRota = relativeRota;
	baseChild.parent = identify(master);
}

void EntityComponentManager::destroy(entity_index_type index) {
	if (index < entityStorageInfo.size() && !entityStorageInfo[index].isDestroyMarked()) {
		assert(entityStorageInfo[index].isValid());
		entityStorageInfo[index].setDestroyMark(true);
		despawnList.push_back(index);
	}
}

void EntityComponentManager::destroy(entity_id id)
{
	destroy(idToIndex[id.id]);
}

void EntityComponentManager::spawnLater(entity_index_type entity)
{
	spawnLaterList.emplace_back(entity);
}

void EntityComponentManager::spawnLater(entity_id id)
{
	spawnLater(idToIndex[id.id]);
}

entity_id EntityComponentManager::identify(entity_index_type entity)
{
	assert(exists(entity));
	if (indexToId.size() != entityStorageInfo.size()) indexToId.resize(entityStorageInfo.size(), 0 );
	if (indexToId[entity] != 0) /* does the handle allready have an id? */ {
		return entity_id(indexToId[entity], idVersion[indexToId[entity]]);
	}
	else {
		// generate id for entity
		if (!freeIdQueue.empty()) {
			// reuse existing index of id vector
			auto id = freeIdQueue.front();
			freeIdQueue.pop_front();
			idToIndex[id] = entity;
			idVersion[id] += 1;	// for every reuse the version gets an increase
			indexToId[entity] = id;
			return entity_id(id, idVersion[id]);
		}
		else {
			// expand id vector
			idToIndex.push_back(entity);
			idVersion.emplace_back(0);
			entity_id_type id = idToIndex.size() - 1;
			indexToId[entity] = id;
			return entity_id(id, 0);
		}
	}
}

void EntityComponentManager::executeDestroys() {
	for (entity_index_type index : despawnList) {
		if (hasComp<Collider>(index) && !hasComp<Movement>(index)) { staticEntitiesChanged = true; }
		// reset id references:
		if (hasID(index)) {
			auto id = indexToId[index];
			freeIdQueue.push_back(id);
			indexToId[index] = 0;
			idToIndex[id] = 0;
		}
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
	childParentDestroy();
	parentChildDestroy();
	despawnList.reserve(entityStorageInfo.size());	// make sure the iterators stay valid
	executeDelayedSpawns();
	deregisterDestroyedEntities();
	executeDestroys();
	std::sort(freeIdQueue.begin(), freeIdQueue.end());
	std::sort(freeIndexQueue.begin(), freeIndexQueue.end());
}

void EntityComponentManager::moveEntity(entity_index_type start, entity_index_type goal)
{
	assert(entityStorageInfo.size() > goal && entityStorageInfo[goal].isValid() == false);
	assert(entityStorageInfo.size() > start && entityStorageInfo[start].isValid() == true); 
	if (hasID(start)) {
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

entity_index_type EntityComponentManager::findBiggestValidHandle()
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
	std::vector<entity_index_type> handleVec;

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

void EntityComponentManager::childParentDestroy()
{
	for (auto& entity : despawnList) {
		if (hasComp<BaseChild>(entity)) {
			despawnList.push_back(getIndex(getComp<BaseChild>(entity).parent));
		}
	}
}

void EntityComponentManager::parentChildDestroy() {
	for (auto& entity : despawnList) {
		if (hasComp<Parent>(entity)) {
			auto& parent = getComp<Parent>(entity);
			for (auto& child : parent.children) {
				despawnList.push_back(getIndex(child));
			}
		}
	}
}

void EntityComponentManager::deregisterDestroyedEntities() {
	for (entity_index_type entity : despawnList) {
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
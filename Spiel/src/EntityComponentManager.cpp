#include "EntityComponentManager.h"

entity_handle EntityComponentManager::create() {
	if (!freeHandleQueue.empty()) {
		entity_handle handle = freeHandleQueue.front();
		freeHandleQueue.pop_front();
		entities[handle].setValid(true);
		entities[handle].setDestroyMark(false);
		entities[handle].setSpawned(false);
		latestHandle = handle;
	}
	else {
		entities.emplace_back( true );
		latestHandle = static_cast<entity_handle>(entities.size() - 1);
	}
	return latestHandle;
}

void EntityComponentManager::link(entity_handle slave, entity_handle master, Vec2 relativePos, float relativeRota)
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

void EntityComponentManager::destroy(entity_handle entitiy_id) {
	if (entitiy_id < entities.size() && !entities[entitiy_id].isDestroyMarked()) {
		assert(entities[entitiy_id].isValid());
		entities[entitiy_id].setDestroyMark(true);
		despawnList.push_back(entitiy_id);
	}
}

void EntityComponentManager::spawnLater(entity_handle entity)
{
	spawnLaterList.emplace_back(entity);
}

entity_id EntityComponentManager::identify(entity_handle entity)
{
	assert(exists(entity));
	if (handleToId.size() != entities.size()) handleToId.resize(entities.size(), 0 );
	if (handleToId[entity] != 0) /* does the handle allready have an id? */ {
		return entity_id(handleToId[entity], idVersion[handleToId[entity]]);
	}
	else {
		// generate id for entity
		if (!freeIdQueue.empty()) {
			// reuse existing index of id vector
			auto id = freeIdQueue.front();
			freeIdQueue.pop_front();
			idToHandle[id] = entity;
			idVersion[id] += 1;	// for every reuse the version gets an increase
			handleToId[entity] = id;
			return entity_id(id, idVersion[id]);
		}
		else {
			// expand id vector
			idToHandle.push_back(entity);
			idVersion.emplace_back(0);
			entity_id_type id = idToHandle.size() - 1;
			handleToId[entity] = id;
			return entity_id(id, handleToId[id]);
		}
	}
}

void EntityComponentManager::executeDestroys() {
	for (entity_handle entity : despawnList) {
		if (hasComp<Collider>(entity) && !hasComp<Movement>(entity)) { staticEntitiesChanged = true; }
		// reset id references:
		if (hasID(entity)) {
			auto id = handleToId[entity];
			freeIdQueue.push_back(id);
			handleToId[entity] = 0;
			idToHandle[id] = 0;
		}
		// reset status of handle:
		entities[entity].setValid(false);
		entities[entity].setDestroyMark(false);
		entities[entity].setSpawned(false);
		freeHandleQueue.push_back(entity);
	}
	despawnList.clear();
}

void EntityComponentManager::update()
{
	childParentDestroy();
	parentChildDestroy();
	despawnList.reserve(entities.size());	// make sure the iterators stay valid
	executeDelayedSpawns();
	deregisterDestroyedEntities();
	executeDestroys();
	std::sort(freeIdQueue.begin(), freeIdQueue.end());
	std::sort(freeHandleQueue.begin(), freeHandleQueue.end());
	defragmentEntities();
}

void EntityComponentManager::moveEntity(entity_handle start, entity_handle goal)
{
	assert(entities.size() > goal && entities[goal].isValid() == false);
	assert(entities.size() > start && entities[start].isValid() == true); 
	if (hasID(start)) {
		idToHandle[handleToId[start]] = goal;
		handleToId[goal] = handleToId[start];
		handleToId[start] = 0;
	}
	for_each(componentStorageTuple, [&](auto& componentStorage) {
		if (componentStorage.contains(start)) {
			componentStorage.insert(goal, componentStorage.get(start));
			componentStorage.remove(start);
			assert(componentStorage.contains(goal));
			assert(!componentStorage.contains(start));
		}
		});
	entities[goal].setValid(true);
	entities[goal].setSpawned(entities[start].isSpawned());
	entities[start].setValid(false);
	entities[start].setDestroyMark(false);
	entities[start].setSpawned(false);
}

entity_handle EntityComponentManager::findBiggestValidHandle()
{
	for (int i = entities.size() - 1; i > 0; i--) {
		if (entities[i].isValid()) return i;
	}
	return 0;
}

void EntityComponentManager::shrink() {
	// !! handles must be sorted !!
	auto lastEl = findBiggestValidHandle();
	entities.resize(lastEl + 1, EntityStatus(false));
	std::vector<entity_handle> handleVec;

	for (int i = freeHandleQueue.size() - 1; i >= 0; i--) {
		if (freeHandleQueue.at(i) < entities.size()) break;
		freeHandleQueue.pop_back();
	}
}

void EntityComponentManager::defragmentEntities()
{
	int maxDefragEntCount;
	float minFragmentation;
	switch (defragMode) {
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
		maxDefragEntCount = memorySize();
		minFragmentation = 0.00001f;
		break;
	}

	if (fragmentation() > minFragmentation) {
		shrink();
		
		for (int defragCount = 0; defragCount < maxDefragEntCount; defragCount++) {
			if (freeHandleQueue.empty()) break;
			auto biggesthandle = findBiggestValidHandle();
			moveEntity(biggesthandle, freeHandleQueue.front());
			freeHandleQueue.pop_front();
			shrink();
		}
	}
}

void EntityComponentManager::childParentDestroy()
{
	for (auto& entity : despawnList) {
		if (hasComp<BaseChild>(entity)) {
			despawnList.push_back(getEntity(getComp<BaseChild>(entity).parent));
		}
	}
}

void EntityComponentManager::parentChildDestroy() {
	for (auto& entity : despawnList) {
		if (hasComp<Parent>(entity)) {
			auto& parent = getComp<Parent>(entity);
			for (auto& child : parent.children) {
				despawnList.push_back(getEntity(child));
			}
		}
	}
}

void EntityComponentManager::deregisterDestroyedEntities() {
	for (entity_handle entity : despawnList) {
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
	return entities.size() - (freeHandleQueue.size() + 1);
}

size_t const EntityComponentManager::memorySize() {
	return entities.size();
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
	return (float)freeHandleQueue.size() / (float)memorySize();
}

float randomFloatd(float MaxAbsVal) {
	float randomNum = rand() % 1'000 / 1'000.0f;
	randomNum -= 0.5f;
	randomNum *= MaxAbsVal;
	return randomNum;
}
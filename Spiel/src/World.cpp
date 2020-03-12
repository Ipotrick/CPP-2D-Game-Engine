#include "World.h"

std::vector<Drawable> World::getDrawableVec()
{
	std::vector<Drawable> res;
	res.reserve(entities.size());
	auto iterB = drawableController.componentData.begin();
	for (auto iterA = entities.begin(); iterA != entities.end(); ++iterA) {
		res.push_back(buildDrawable(iterA->second, iterB->second));
		++iterB;
	}
	return res;
}

std::vector<std::tuple<uint32_t, Collidable*>> World::getCollidablePtrVec()
{
	std::vector<std::tuple<uint32_t, Collidable*>> res;
	res.reserve(entities.size());
	for (auto& el : entities) {
		res.push_back({ el.first, el.second.getCollidablePtr() });
	}
	return res;
}

void World::spawnEntity(Entity ent_, CompDataDrawable draw_)
{
	entities.insert( {nextID, ent_ });
	drawableController.registerEntity( nextID, draw_);
	nextID++;
}

void World::despawn(uint32_t entitiy_id)
{
	auto iter = entities.find(entitiy_id);
	if (iter != entities.end()) {
		despawnList.push_back(entitiy_id);
	}
}

void World::executeDespawns()
{
	for (int entitiy_id : despawnList) {
		auto iter = entities.find(entitiy_id);
		if (iter != entities.end()) {
			entities.erase(iter);
		}
	}
	despawnList.clear();
}

void World::deregisterDespawnedEntities()
{
	for (int entitiy_id : despawnList) {
		auto iter = entities.find(entitiy_id);
		if (iter != entities.end()) {
			mortalController.deregisterEntity(iter->first);
			playerController.deregisterEntity(iter->first);
			bulletController.deregisterEntity(iter->first);
		}
	}
}
#include "World.h"

std::vector<Drawable> World::getDrawableVec()
{
	std::vector<Drawable> res;
	res.reserve(entities.size());
	auto iterB = drawableCompCtrl.componentData.begin();
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

void World::spawnEntity(Entity const& ent_, CompDataDrawable const& draw_)
{
	entities.insert( {nextID, ent_ });
	drawableCompCtrl.registerEntity( nextID, draw_);
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
			drawableCompCtrl.deregisterEntity(iter->first);
			playerCompCtrl.deregisterEntity(iter->first);
			healthCompCtrl.deregisterEntity(iter->first);
			ageCompCtrl.deregisterEntity(iter->first);
			bulletCompCtrl.deregisterEntity(iter->first);
		}
	}
}
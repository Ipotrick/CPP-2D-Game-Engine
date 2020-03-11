#include "World.h"

std::vector<Drawable> World::getDrawableVec()
{
	std::vector<Drawable> res;
	res.reserve(entities.size());
	for (auto& el : entities) {
		res.push_back(el.second.getDrawable());
	}
	return res;
}

std::vector<Collidable*> World::getCollidablePtrVec()
{
	std::vector<Collidable*> res;
	res.reserve(entities.size());
	for (auto& el : entities) {
		res.emplace_back(el.second.getCollidablePtr());
	}
	return res;
}

void World::spawnEntity(Entity ent_)
{
	entities.insert({ ent_.getId(), ent_ });
	latestID = ent_.getId();
}

void World::despawn(int entitiy_id)
{
	auto entPtr = getEntityPtr(entitiy_id);
	if (entPtr != nullptr) {
		despawn(*entPtr);
	}
}

void World::despawn(Entity & entity_)
{
	entity_.despawned = true;
	despawnList.emplace_back(entity_.getId());
}

std::vector<int> const& World::getDespawnIDs()
{
	return despawnList;
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

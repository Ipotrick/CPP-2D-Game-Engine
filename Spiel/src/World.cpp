#include "World.h"

std::vector<Drawable> World::getDrawableVec()
{
	std::vector<Drawable> res;
	res.reserve(entities.size());
	for (auto& el : entities) {
		res.push_back(el.getDrawable());
	}
	return res;
}

std::vector<Collidable*> World::getCollidablePtrVec()
{
	std::vector<Collidable*> res;
	res.reserve(entities.size());
	for (auto& el : entities) {
		res.emplace_back(el.getCollidablePtr());
	}
	return res;
}

void World::spawnEntity(Entity ent_)
{
	entities.emplace_back(ent_);
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



void World::executeDespawns()
{
	for (int entitiy_id : despawnList) {
		for (auto iter = entities.begin(); iter != entities.end(); iter++) {
			if (iter->getId() == entitiy_id) {
				entities.erase(iter);
				break;
			}
		}
	}
	despawnList.clear();
}

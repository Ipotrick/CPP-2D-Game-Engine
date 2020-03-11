#pragma once
#include <vector>

#include "robin_hood.h"

#include "Entity.h"


class World {
public:
	World() : latestID{0}
	{
	}
	
	/* returns entity with the given if IF it exists, otherwise a nullptr is returned, O(1) */
	Entity * const getEntityPtr(uint32_t id_);
	/* creates Copy of given entity and pushes it into the entitiy vector, O(1) */
	void spawnEntity(Entity ent_);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(int entitiy_id);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(Entity & entity_);
	/* returns const refference to entities that are going to be deleted */
	std::vector<int> const& getDespawnIDs();

	/* INNER ENGINE FUNCTION */
	void executeDespawns();
	/* INNER ENGINE FUNCTION */
	std::vector<Drawable> getDrawableVec();
	/* INNER ENGINE FUNCTION */
	std::vector<Collidable*> getCollidablePtrVec();
public:
	uint32_t latestID;
	robin_hood::unordered_map<uint32_t, Entity> entities;
private:
	std::vector<int> despawnList;
};

inline Entity *const World::getEntityPtr(uint32_t id_) {
	auto entIter = entities.find(id_);
	if (entIter != entities.end() && entIter->first == id_) {
		return &entIter->second;
	}
	else {
		return nullptr;
	}
}
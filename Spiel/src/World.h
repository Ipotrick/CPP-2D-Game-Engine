#pragma once

#include "Entity.h"
#include <vector>

class World {
public:
	World()
	{
	}
	
	/* returns entity with the given if IF it exists, otherwise a nullptr is returned, O(log2(n)) */
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
	std::vector<Entity> entities;
private:
	std::vector<int> despawnList;
};

inline Entity *const World::getEntityPtr(uint32_t id_) {
	auto entIter = std::lower_bound(entities.begin(), entities.end(), id_,
		[](Entity a, uint32_t b)
		{
			return a.getId() < b;
		}
	);
	if (entIter != entities.end() && entIter->getId() == id_) {
		return &*entIter;
	}
	else {
		return nullptr;
	}
}
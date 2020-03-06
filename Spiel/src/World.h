#pragma once

#include "Entity.h"
#include <vector>

class World {
public:
	World()
	{
	}

	std::vector<Drawable> getDrawableVec();
	std::vector<Collidable*> getCollidablePtrVec();
	/* 
	@returns entity with the given if IF it exists, otherwise a nullptr is returned
	@runntime: O(log2(n))
	*/
	Entity * const getEntityPtr(uint32_t id_);
	/* creates Copy of given entity and pushes it into the entitiy vector */
	void spawnEntity(Entity ent_);
	/* marks entity for deletion, entities are deleted after each update */
	void despawn(int entitiy_id);
	/* marks entity for deletion, entities are deleted after each update */
	void despawn(Entity & entity_);

	void executeDespawns();
public:
	std::vector<Entity> entities;
private:
	std::vector<int> despawnList;
};

inline Entity *const World::getEntityPtr(uint32_t id_) {
	/*
	int pivot = ceilf( entities.size() / 2.0 );
	int oldPivot = 0;
	while (oldPivot != pivot) {
		oldPivot = pivot;
		auto id = entities.at(pivot).getId();

		if (id == id_) {
			if (entities.at(pivot).despawned == false) {
				return &entities.at(pivot);
			}
			else {
				return nullptr; 
			}
		}
		else {
			if (id < id_) {
				pivot = pivot + ceil(pivot / 2.0);
			}
			else {
				pivot = pivot - ceil(pivot / 2.0);
			}
		}
	}*/
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
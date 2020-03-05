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
	Entity * const getEntityPtr(uint32_t id_);
	void spawnEntity(Entity ent_);
	void despawn(int entitiy_id);
	void despawn(Entity & entity_);

	void executeDespawns();
public:
	std::vector<Entity> entities;
private:
	std::vector<int> despawnList;
};

inline Entity *const World::getEntityPtr(uint32_t id_) {
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
	}
	return nullptr;
}
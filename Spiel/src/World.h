#pragma once

#include "Entity.h"
#include <vector>

class World {
public:
	World() {}

	std::vector<Drawable> getDrawableVec();
	std::vector<Collidable*> getCollidablePtrVec();
	void spawnEntity(Entity ent_);
	void removeEntity(int entitiy_id);
public:
	std::vector<Entity> entities;
};
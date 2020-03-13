#pragma once
#include <vector>

#include "robin_hood.h"

#include "GameComponents.h"
#include "Entity.h"


class World {
public:
	World() : nextID{ 0 }, despawnList{}
	{
	}
	
	/* returns entity with the given if IF it exists, otherwise a nullptr is returned, O(1) */
	Entity * const getEntityPtr(uint32_t id_);
	/* creates Copy of given entity and pushes it into the entitiy vector, O(1) */
	void spawnEntity(Entity const& ent_, CompDataDrawable const& draw_);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(uint32_t entitiy_id);
	/* returnes the last ID */
	uint32_t const getLastID();

private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void deregisterDespawnedEntities();	//CALL BEFORE "executeDespawns"
	void executeDespawns();
	Drawable buildDrawable(Entity const& ent_, CompDataDrawable const& draw_);
	std::vector<Drawable> getDrawableVec();
	std::vector<std::tuple<uint32_t, Collidable*>> getCollidablePtrVec();
public:
	robin_hood::unordered_map<uint32_t, Entity> entities;
	/* ComponentController list */
	CompController<CompDataDrawable>	drawableCompCtrl;
	CompController<CompDataHealth>		healthCompCtrl;
	CompController<CompDataAge>			ageCompCtrl;
	CompController<CompDataPlayer>		playerCompCtrl;
	CompController<CompDataBullet>		bulletCompCtrl;
private:
	uint32_t nextID;
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

inline uint32_t const World::getLastID() {
	assert(nextID > 0);	//DO NOT CALL THIS FUNCTION WITH 0 ENTITIES
	return nextID -1;
}


inline Drawable World::buildDrawable(Entity const& ent_, CompDataDrawable const& draw_)
{
	return Drawable(ent_.position, draw_.drawingPrio, draw_.scale, draw_.color, (Form)draw_.form, ent_.rotation);
}
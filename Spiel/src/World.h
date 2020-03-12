#pragma once
#include <vector>

#include "robin_hood.h"

#include "GameComponents.h"
#include "Entity.h"


class World {
public:
	World() : nextID{0}
	{
	}
	
	/* returns entity with the given if IF it exists, otherwise a nullptr is returned, O(1) */
	Entity * const getEntityPtr(uint32_t id_);
	/* creates Copy of given entity and pushes it into the entitiy vector, O(1) */
	void spawnEntity(Entity ent_, CompDataDrawable draw_);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(uint32_t entitiy_id);
	/* returnes the last ID */
	uint32_t const& getLastID();

private:
	friend class Engine;
	/* INNER ENGINE FUNCTIONS: */
	void deregisterDespawnedEntities();	//CALL BEFORE EXECUTE DESPAWNS
	void executeDespawns();
	Drawable buildDrawable(Entity const& ent_, CompDataDrawable const& draw_);
	std::vector<Drawable> getDrawableVec();
	std::vector<std::tuple<uint32_t, Collidable*>> getCollidablePtrVec();
public:
	robin_hood::unordered_map<uint32_t, Entity> entities;
	CompController<CompDataDrawable> drawableController;
	CompController<CompDataMortal> mortalController;
	CompController<CompDataPlayer> playerController;
	CompController<CompDataBullet> bulletController;
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

inline uint32_t const& World::getLastID() {
	return nextID -1;
}


inline Drawable World::buildDrawable(Entity const& ent_, CompDataDrawable const& draw_)
{
	return Drawable(ent_.position, draw_.drawingPrio, draw_.scale, draw_.color, (Drawable::Form)draw_.form, ent_.rotation);
}
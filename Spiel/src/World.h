#pragma once
#include <algorithm>

#include <vector>
#include <queue>

#include "robin_hood.h"

#include "CoreComponents.h"
#include "GameComponents.h"
#include "Entity.h"
#include "glmath.h"


struct Light {
	Light(vec2 pos, float rad, uint32_t id_, vec4 col) : position{pos}, radius{rad}, id{id_}, color{col} {}
	vec2 position;
	float radius;
	uint32_t id;
	vec4 color;
};


class World {
public:
	World() : nexBacktID{ 1 }, lastID{ 0 }, despawnList{}
	{
		entities.push_back({ false, Entity(0,0, Collidable(vec2(0,0), Form::CIRCLE, false, false)) });
	}
	
	/* returns entity with the given if IF it exists, otherwise a nullptr is returned, O(1) */
	Entity * const getEntityPtr(uint32_t id_);
	/* returns reference to the entitiy with the given id. UNCHECKED, O(1) */
	Entity& getEntity(uint32_t id);
	/* returnes if entitiy exists or not */
	bool doesEntExist(uint32_t id);
	/* creates Copy of given entity inside the entity map, O(1) */
	void spawnEntity(Entity const& ent, CompDataDrawable const& draw);
	/* creates Copy of given entity inside the entity map, O(1) */
	void spawnSolidEntity(Entity ent, CompDataDrawable const& draw, CompDataSolidBody solid);
	/* adds a slave entity to an owner entity */
	void spawnSlave(Entity ent, CompDataDrawable const& draw, uint32_t ownerID, vec2 relativePos, float relativeRota);
	/* adds a slave entity to an owner entity, that contributes to the entities hitbox */
	void spawnSolidSlave(Entity ent, CompDataDrawable const& draw, uint32_t ownerID, vec2 relativePos, float relativeRota);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(uint32_t entitiy_id);
	/* returnes the id of the most rescently spawned entity .
		if 0 is returnsed, there are no entities spawned yet */
	uint32_t const getLastID();
	/* returns count of entities */
	uint32_t const getEntCount();
	/* returns the size of the vector that holds the entities */
	uint32_t const getEntMemSize();

private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void deregisterDespawnedEntities();	//CALL BEFORE "executeDespawns"
	void executeDespawns();
	Drawable buildDrawable(uint32_t id_, Entity const& ent_, CompDataDrawable const& draw_);
	std::vector<Drawable> getDrawableVec();
	Light buildLight(uint32_t id, Entity const& ent_, CompDataLight const& light_);
	std::vector<Light> getLightVec();
	std::vector<std::tuple<uint32_t, Collidable*>> getCollidablePtrVec();
public:
	/* engine ComponentController list */
	CompControllerLUT<CompDataSolidBody> solidBodyCompCtrl;
	CompControllerLUT<CompDataDrawable>	drawableCompCtrl;
	CompController< CompDataComposit4> composit4CompCtrl;
	/* game ComponentController list */
	CompController<CompDataLight>		lightCompCtrl;
	CompController<CompDataHealth>		healthCompCtrl;
	CompController<CompDataAge>			ageCompCtrl;
	CompController<CompDataPlayer>		playerCompCtrl;
	CompController<CompDataBullet>		bulletCompCtrl;
private:
	std::vector<std::pair<bool, Entity>> entities;
	std::queue<uint32_t> emptySlots;
	uint32_t nexBacktID;
	uint32_t lastID;
	std::vector<int> despawnList;
};

inline bool World::doesEntExist(uint32_t id) {
	assert(id < entities.size());
	return entities[id].first;
}

inline Entity *const World::getEntityPtr(uint32_t id_) {
	assert(id_ < entities.size());
	if (entities[id_].first) {
		return &entities[id_].second;
	}
	else {
		return nullptr;
	}
}

inline Entity& World::getEntity(uint32_t id) {
	assert(id < entities.size());
	assert(doesEntExist(id));
	return entities[id].second;
}

inline uint32_t const World::getLastID() {
	return lastID;
}


inline Drawable World::buildDrawable(uint32_t id_, Entity const& ent_, CompDataDrawable const& draw_)
{
	return Drawable(id_, ent_.position, draw_.drawingPrio, draw_.scale, draw_.color, (Form)draw_.form, ent_.rotation, draw_.throwsShadow);
}
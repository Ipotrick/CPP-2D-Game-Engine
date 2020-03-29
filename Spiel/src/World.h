#pragma once
#include <algorithm>
#include <fstream>

#include <vector>
#include <queue>

#include "robin_hood.h"
#include "json.h"

#include "CoreComponents.h"
#include "GameComponents.h"
#include "Entity.h"
#include "glmath.h"

#define generateComponentAccessFunctionsLUT(CompName, CtrlName) \
template<> inline CompName& World::getComp<CompName>(uint32_t id) { return CtrlName.getComponent(id); }\
template<> inline CompName* const World::getCompPtr<CompName>(uint32_t id) { return CtrlName.getComponentPtr(id); }\
template<> inline bool World::hasComp<CompName>(uint32_t id) { return CtrlName.isRegistered(id); }\
template<> inline void World::addComp<CompName>(uint32_t id, CompName data) { CtrlName.registerEntity(id, data); }

#define generateComponentAccessFunctions(CompName, CtrlName) \
template<> inline robin_hood::unordered_map<uint32_t, CompName>& World::getAllComps<CompName>() { return CtrlName.componentData; }\
template<> inline CompController<CompName>& World::getCtrl<CompName>() { return CtrlName; } \
template<> inline CompName& World::getComp<CompName>(uint32_t id) { return CtrlName.getComponent(id); }\
template<> inline CompName* const World::getCompPtr<CompName>(uint32_t id) { return CtrlName.getComponentPtr(id); }\
template<> inline bool World::hasComp<CompName>(uint32_t id) { return CtrlName.isRegistered(id); }\
template<> inline void World::addComp<CompName>(uint32_t id, CompName data) { CtrlName.registerEntity(id, data); }

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
	
	/* entity access utility */
	/* returns entity with the given if IF it exists, otherwise a nullptr is returned, O(1) */
	Entity * const getEntityPtr(uint32_t id_);
	/* returns reference to the entitiy with the given id. UNCHECKED, O(1) */
	Entity& getEntity(uint32_t id);
	/* returnes if entitiy exists or not, O(1) */
	bool doesEntExist(uint32_t id);

	/* entity create/destruct utility */
	/* creates Copy of given entity inside the entity map, O(1) */
	void spawnEntity(Entity const& ent, Draw const& draw);
	/* creates Copy of given entity inside the entity map, O(1) */
	void spawnSolidEntity(Entity ent, Draw const& draw, SolidBody solid);
	/* adds a slave entity to an owner entity */
	void spawnSlave(Entity ent, Draw const& draw, uint32_t ownerID, vec2 relativePos, float relativeRota);
	/* adds a slave entity to an owner entity, that contributes to the entities hitbox */
	void spawnSolidSlave(Entity ent, Draw const& draw, uint32_t ownerID, vec2 relativePos, float relativeRota);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(uint32_t entitiy_id);

	/* world utility */
	/* returnes the id of the most rescently spawned entity .
		if 0 is returnsed, there are no entities spawned yet, O(1) */
	uint32_t const getLastID();
	/* returns count of entities, O(1) */
	size_t const getEntCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const getEntMemSize();

	/* Component access utility */
	/* returns refference to the component controller, O(1) */
	template<typename ComponentType> CompController<ComponentType>& getCtrl();
	/* returnes container of all component data of a component, O(1) */
	template<typename ComponentType> robin_hood::unordered_map<uint32_t, ComponentType>& getAllComps();
	/* returnes refference the component data of one entitiy, O(1) */
	template<typename ComponentType> ComponentType & getComp(uint32_t id);
	/* returnes constant pointer to the component data of one entity, O(1) */
	template<typename ComponentType> ComponentType * const getCompPtr(uint32_t id);
	/* returns bool whether or not the given entity has the component, O(1) */
	template<typename ComponentType> bool hasComp(uint32_t id);
	/* registeres a new component under the given id, O(1) (can be slow) */
	template<typename ComponentType> void addComp(uint32_t id, ComponentType data);

	void loadMap(std::string_view);

private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void slaveOwnerDespawn(); // slaves with dead owner get despawned, dead slaves cut their refference of themselfes to the owner
	void deregisterDespawnedEntities();	// CALL BEFORE "executeDespawns"
	void executeDespawns();
	Drawable buildDrawable(uint32_t id_, Entity const& ent_, Draw const& draw_);
	std::vector<Drawable> getDrawableVec();
	Light buildLight(uint32_t id, Entity const& ent_, CompDataLight const& light_);
	std::vector<Light> getLightVec();
	std::vector<std::tuple<uint32_t, Collidable*>> getCollidablePtrVec();
private:
	/* DO NOT USE THESE DIRECTLY! USE THE COMPONENT UTILITY TEMPLATES */
	/* engine ComponentController list */
	CompControllerLUT<SolidBody>	solidBodyCompCtrl;
	CompControllerLUT<Draw>			drawableCompCtrl;
	CompController<Composit<4>>		composit4CompCtrl;
	/* game ComponentController list */
	CompController<CompDataLight>	lightCompCtrl;
	CompController<Health>			healthCompCtrl;
	CompController<Age>				ageCompCtrl;
	CompController<Player>			playerCompCtrl;
	CompController<Bullet>			bulletCompCtrl;

	std::vector<std::pair<bool, Entity>> entities;
	std::queue<uint32_t> emptySlots;
	uint32_t nexBacktID;
	uint32_t lastID;
	std::vector<int> despawnList;
};

generateComponentAccessFunctionsLUT(SolidBody, solidBodyCompCtrl)
generateComponentAccessFunctionsLUT(Draw, drawableCompCtrl)

generateComponentAccessFunctions(Composit<4>, composit4CompCtrl)
generateComponentAccessFunctions(CompDataLight, lightCompCtrl)
generateComponentAccessFunctions(Age, ageCompCtrl)
generateComponentAccessFunctions(Player, playerCompCtrl)
generateComponentAccessFunctions(Bullet, bulletCompCtrl)
generateComponentAccessFunctions(Health, healthCompCtrl)

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


inline Drawable World::buildDrawable(uint32_t id_, Entity const& ent_, Draw const& draw_)
{
	return Drawable(id_, ent_.position, draw_.drawingPrio, draw_.scale, draw_.color, (Form)draw_.form, ent_.rotation, draw_.throwsShadow);
}
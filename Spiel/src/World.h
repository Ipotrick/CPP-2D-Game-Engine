#pragma once
#include <algorithm>
#include <fstream>

#include <vector>
#include <queue>

#include "robin_hood.h"
#include "json.h"

#include "ECS.h"
#include "CoreComponents.h"
#include "GameComponents.h"
#include "Entity.h"
#include "glmath.h"

struct Light {
	Light(vec2 pos, float rad, uint32_t id_, vec4 col) : position{ pos }, radius{ rad }, id{ id_ }, color{ col } {}
	vec2 position;
	float radius;
	uint32_t id;
	vec4 color;
};

#define GENERATE_COMPONENT_ACCESS_FUNCTIONS_INTERN(CompType, CompStorage, storageType) \
template<> inline auto& view<CompType>() { return CompStorage; } \
template<> inline CompType& getComp<CompType>(uint32_t id) { return CompStorage.getComponent(id); }\
template<> inline bool hasComp<CompType>(uint32_t id) { return CompStorage.isRegistrated(id); }\
template<> inline void addComp<CompType>(uint32_t id, CompType data) { CompStorage.registrate(id, data); } \
template<> inline void addComp<CompType>(uint32_t id) { CompStorage.registrate(id, CompType()); }

#define generateComponentAccessFunctionsExtern(CompType, CompStorage, storageType) \
template<> inline auto& World::view<CompType>() { return CompStorage; } \
template<> inline CompType& World::getComp<CompType>(uint32_t id) { return CompStorage.getComponent(id); }\
template<> inline bool World::hasComp<CompType>(uint32_t id) { return CompStorage.isRegistrated(id); }\
template<> inline void World::addComp<CompType>(uint32_t id, CompType data) { CompStorage.registrate(id, data); } \
template<> inline void World::addComp<CompType>(uint32_t id) { CompStorage.registrate(id, CompType()); }

#define GENERATE_COMPONENT_CODE(CompType, StorageType, Num) \
private: ComponentStorage<CompType, StorageType> compStorage ## Num; \
public: GENERATE_COMPONENT_ACCESS_FUNCTIONS_INTERN(CompType, compStorage ## Num, storageType)


struct Ent {
	Ent(bool valid_ = false) : valid{ valid_ } {}
	bool valid;
};

class World {
public:

	World() : lastID{ 0 }, despawnList{}
	{
		entities.push_back({ false });
	}
	
	/* entity access utility */
	/* returnes if entitiy exists or not, O(1) */
	bool doesEntExist(ent_id_t id);
	/* entity create/destruct utility */
	/* creates blank entity and returns its id */
	ent_id_t createEnt();
	/* enslaves the first ent to the second */
	void enslaveEntTo(ent_id_t slave, ent_id_t owner, vec2 relativePos, float relativeRota);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(ent_id_t id);

	/* world utility */
	/* returnes the id of the most rescently spawned entity.
		if 0 is returnsed, there are no entities spawned yet, O(1) */
	ent_id_t const getLastID();
	/* returns count of entities, O(1) */
	size_t const getEntCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const getEntMemSize();

	/* Component access utility */
	/* returnes reference to a safe virtual container or "view" to the given components */
	template<typename CompType> auto& view();
	/* returnes refference the component data of one entitiy, O(1) */
	template<typename CompType> CompType& getComp(ent_id_t id);
	/* returns bool whether or not the given entity has the component added/registered, O(1) */
	template<typename CompType> bool hasComp(ent_id_t id);
	/* registeres a new component under the given id, O(1) */
	template<class CompType> void addComp(ent_id_t id, CompType data);
	/*registeres a new component under the given id, O(1) */
	template<class CompType> void addComp(ent_id_t id);

	void loadMap(std::string);
private:
	GENERATE_COMPONENT_CODE(Base, storage_index_t, 0)
	GENERATE_COMPONENT_CODE(Movement, storage_index_t, 1)
	GENERATE_COMPONENT_CODE(Collider, storage_index_t, 2)
	GENERATE_COMPONENT_CODE(SolidBody, storage_index_t, 3)
	GENERATE_COMPONENT_CODE(Draw, storage_index_t, 4)
	GENERATE_COMPONENT_CODE(Slave, storage_index_t, 5)
	GENERATE_COMPONENT_CODE(Composit<4>, storage_index_t, 6)
	GENERATE_COMPONENT_CODE(CompDataLight, storage_index_t, 7)
	GENERATE_COMPONENT_CODE(Health, storage_index_t, 8)
	GENERATE_COMPONENT_CODE(Age, storage_index_t, 9)
	GENERATE_COMPONENT_CODE(Player, storage_index_t, 10)
	GENERATE_COMPONENT_CODE(Bullet, storage_index_t, 11)
private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void slaveOwnerDespawn(); // slaves with dead owner get despawned, dead slaves cut their refference of themselfes to the owner
	void deregisterDespawnedEntities();	// CALL BEFORE "executeDespawns"
	void executeDespawns();
	Drawable buildDrawable(ent_id_t id, Draw const& draw);
	std::vector<Drawable> getDrawableVec();
private:

	std::vector<Ent> entities;
	std::queue<ent_id_t> emptySlots;
	ent_id_t lastID;
	std::vector<ent_id_t> despawnList;
	bool staticSpawnOrDespawn{ false };
};


inline bool World::doesEntExist(ent_id_t id) {
	assert(id < entities.size());
	return entities[id].valid;
}

inline ent_id_t const World::getLastID() {
	return lastID;
}

inline Drawable World::buildDrawable(ent_id_t id, Draw const& draw)
{
	assert(id > 0);
	return Drawable(id, getComp<Base>(id).position, draw.drawingPrio, draw.scale, draw.color, (Form)draw.form, getComp<Base>(id).rotation, draw.throwsShadow);
}
#pragma once
#include <algorithm>
#include <fstream>

#include <vector>
#include <queue>
#include <bitset>

#include "robin_hood.h"
#include "json.h"

#include "BaseTypes.h"
#include "RenderTypes.h"
#include "algo.h"

#include "ECS.h"
#include "CoreComponents.h"
#include "GameComponents.h"
#include "CoreSystemUniforms.h"
#include "GameSystemUnifroms.h"

#include "GameSystemWorldInclude"

// GameSystemWorldInclude API
#define SYSTEM_UNIFORMS(System) SystemUniforms ## System uniforms ## System;
#include "GameSystemWorldInclude"

#define CORE_COMPONENT_COUNT 10
// GameCompWorldInclude API
	#define COMPONENT_INDEX(CompType, Index) template<> struct ComponentIndex<CompType> { static int constexpr index = Index + CORE_COMPONENT_COUNT; };
	#define COMPONENT_STORAGE(CompType, StorageType) ComponentStorage<CompType, StorageType>
#include "GameCompWorldInclude"

#define CORE_COMPONENT_INDEX(CompType, Index) template<> struct ComponentIndex<CompType> { static int constexpr index = Index; };

class EntityStatus {
public:
	EntityStatus(bool valid) : flags{} {
		flags[0] = valid;
		flags[1] = false;
		flags[2] = false;
	}
	__forceinline void setValid(bool valid) {  flags[0] = valid; }
	__forceinline bool isValid() { return flags[0]; }
	__forceinline void setDestroyMark(bool mark) { flags[1] = mark; }
	__forceinline bool isDestroyMarked() { return flags[1]; }
	__forceinline void setSpawned(bool spawned) { flags[2] = spawned; }
	__forceinline bool isSpawned() { return flags[2]; }

private:
	// flag 0: valid
	// flag 1: destroyMark
	// flag 2: spawned
	std::bitset<3> flags;
};

template<typename First, typename Second, typename ... CompTypes>
class MultiView;
template<typename CompType>
class SingleView;
class ComponentView;

class World {
	template<typename First, typename Second, typename ... CompTypes>
	friend class MultiView;
	template<typename CompType>
	friend class SingleView;
public:

	World() : latestHandle{ 0 }, defragMode{ DefragMode::NONE }, despawnList{}
	{
		entities.push_back({ false });
		handleToId.push_back(entity_id(0));
		idToHandle.push_back(0);
	}
	
	/* returnes if entitiy exists or not, O(1) */
	bool doesEntExist(entity_handle entity);
	/* entity create/destruct utility */
	/* creates blank entity and returns its entity, O(1) */
	entity_handle createEnt();
	/* enslaves the first ent to the second, ~O(1) */
	void linkBase(entity_handle slave, entity_handle owner, Vec2 relativePos, float relativeRota);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void destroy(entity_handle entity);
	/* returns if an entitiy is related to another entity via a slave/owner relationship */
	bool areEntsRelated(entity_handle collID, entity_handle otherID);
	/* immediately makes entity invisible for systems and scripts but the entity stays in memory so it can be respawned */
	void despawn(entity_handle entity);
	/* immediately respawnes despawned entity */
	void respawn(entity_handle entity);
	/* immediate spawn of entitiy */
	void spawn(entity_handle entity);
	/* delayed(after update call) spawn of an entity */
	void spawnLater(entity_handle entity);
	/* delayed(after update call) respawn of an entity */
	void respawnLater(entity_handle entity);

	/* identification utility */
	/* returns true when entity has an id */
	bool hasID(entity_handle entity);
	/* generates new id for entity or returns existing id */
	entity_id identify(entity_handle entity);
	/* if entity has an id it will be returned, call hasID before! */
	entity_id getID(entity_handle entity);
	/* returns handle of the entity, call isIdValid before! */
	entity_handle getEnt(entity_id entityID);
	bool isIDValid(entity_id entityID);

	/* world utility */
	/* returnes the entity of the most rescently spawned entity.
		if 0 is returnsed, there are no entities spawned yet.
		Try to not use this function and use the return value of create() instead
		, O(1) */
	entity_handle const getLastEntity();
	/* returns count of entities, O(1) */
	size_t const getEntCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const getEntMemSize();
	/* sets static entities changed flag */
	void staticsChanged();
	/* returns wheter or not static entities changed */
	bool didStaticsChange();
	/* returnes how fragmented the entities are */
	float getFragmentation();
	enum class DefragMode {
		NONE,
		LAZY,
		MODERATE,
		EAGER,
		AGRESSIVE,
		COMPLETE,
		FAST
	};
	/* sets how much at a time and at what fragmentation(%) the world defragments */
	void setDefragMode(DefragMode mode);

	/* Component access utility */
	/* returnes reference to a safe virtual container of the given components one can iterate over.
	the iterator also holds the entity entity of the compoenent it points to, O(1) */
	template<typename CompType> auto& getAll();
	/* returnes refference the component data of one entitiy, ~O(1) */
	template<typename CompType> CompType& getComp(entity_handle entity);
	/* returns bool whether or not the given entity has the component added/registered, ~O(1) */
	template<typename CompType> bool hasComp(entity_handle entity);
	template<typename ... CompTypes> bool hasComps(entity_handle entity);
	/* returns bool whether or not the given entity has'nt the component added/registered, ~O(1) */
	template<typename CompType> bool hasntComp(entity_handle entity);
	template<typename ... CompTypes> bool hasntComps(entity_handle entity);
	/* adds a new Compoenent to an entity, ~O(1) */
	template<typename CompType> CompType& addComp(entity_handle entity, CompType data);
	/* adds a new Compoenent to an entity, ~O(1) */
	template<typename CompType> CompType& addComp(entity_handle entity);
	/* removes a component from the entity */
	template<typename CompType> void remComp(entity_handle entity);
	/* returnes a View, and iterable object that only iterates over the entities with the given Components, ignored despawned entities */
	template<typename First, typename Second, typename ... CompTypes> MultiView<First, Second, CompTypes...> view();
	template<typename CompType> SingleView<CompType> view();
	ComponentView viewComps(entity_handle entity);

	void loadMap(std::string);
public:
	// System Uniform Data
	SYSTEM_UNIFORMS(Physics)
	SYSTEM_UNIFORM_SEGMENT
private:
	// Component Data
private:
	template<typename CompType>
	struct CompStorageIndex {};
	std::tuple<
		ComponentStorage<Base, direct_indexing>,
		ComponentStorage<Movement, direct_indexing>,
		ComponentStorage<Collider, direct_indexing>,
		ComponentStorage<PhysicsBody, direct_indexing>,
		ComponentStorage<LinearEffector, hashing>,
		ComponentStorage<FrictionEffector, hashing>,
		ComponentStorage<Draw, direct_indexing>,
		ComponentStorage<TextureRef, direct_indexing>,
		ComponentStorage<Parent, hashing>,
		ComponentStorage<BaseChild, hashing>,
		COMPONENT_SEGMENT
	> componentStorageTuple;
private:
	template<typename CompType>
	struct ComponentIndex {
		static int const index = -1;
	};
	CORE_COMPONENT_INDEX(Base, 0)
	CORE_COMPONENT_INDEX(Movement, 1)
	CORE_COMPONENT_INDEX(Collider, 2)
	CORE_COMPONENT_INDEX(PhysicsBody, 3)
	CORE_COMPONENT_INDEX(LinearEffector, 4)
	CORE_COMPONENT_INDEX(FrictionEffector, 5)
	CORE_COMPONENT_INDEX(Draw, 6)
	CORE_COMPONENT_INDEX(TextureRef, 7)
	CORE_COMPONENT_INDEX(Parent, 8)
	CORE_COMPONENT_INDEX(BaseChild, 9)
	COMPONENT_INDEX_SEGMENT
private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void tick();
	void moveEntity(entity_handle start, entity_handle goal);
	entity_handle findBiggestValidHandle();
	void shrink(); // shorten entity array and delete freeHandles at the end of the entity array
	void defragmentEntities();
	void childParentDestroy(); // destroy on parent calls destroy on child
	void parentChildDestroy(); // destroy on child calls destroy on parent
	void deregisterDestroyedEntities();
	void executeDelayedSpawns();
	void executeDestroys();
	void sortFreeHandleQueue();
	void sortFreeIDQueue();

	/* resets static entities changed flag*/
	void resetStaticsChangedFlag();
private:
	std::vector<EntityStatus> entities;
	std::queue<entity_handle> freeHandleQueue;
	entity_handle latestHandle;

	std::vector<entity_handle> idToHandle;
	std::vector<uint32_t> idVersion;
	std::vector<entity_id> handleToId;
	std::queue<entity_id> freeIdQueue;

	std::vector<entity_handle> despawnList;
	std::vector<entity_handle> spawnLaterList;
	bool staticEntitiesChanged{ true };

	DefragMode defragMode;
};

// ---- Component Accessors implementations --------------------------------

template<typename CompType> __forceinline auto& World::getAll() { 
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple);
} 

template<typename CompType> __forceinline CompType& World::getComp(entity_handle entity) {
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).get(entity);
} 

template<typename CompType> __forceinline bool World::hasComp(entity_handle entity) {
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).contains(entity);
} 

template<typename CompType> __forceinline bool World::hasntComp(entity_handle entity) {
	return !std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).contains(entity);
} 

template<typename CompType> __forceinline CompType& World::addComp(entity_handle entity, CompType data) {
	std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).insert(entity, data);
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple)[entity];
} 

template<typename CompType> __forceinline CompType& World::addComp(entity_handle entity) {
	std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).insert(entity, CompType());
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple)[entity];
} 

template<typename CompType> __forceinline void World::remComp(entity_handle entity) {
	std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).remove(entity);
}

// ---------- hasComps implementation --------------------------------------

namespace _HasCompsTesterImpl {
	template<typename... CompTypes>
	struct HasCompsTester {
		HasCompsTester(entity_handle entity, World& world) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasCompsTester<Head, CompTypes...> {
		HasCompsTester(entity_handle entity, World& world) {
			if (world.hasComp<Head>(entity)) {
				HasCompsTester<CompTypes...> recursiveTester(entity, world);
				result = recursiveTester.result;
			}
			else {
				result = false;
			}
		}
		bool result;
	};
}

template<typename... CompTypes>
inline bool World::hasComps(entity_handle entity) {
	_HasCompsTesterImpl::HasCompsTester<CompTypes...> tester(entity, *this);
	return tester.result;
}

// --------- hasntComps implementation -------------------------------------

namespace _HasntCompsTesterImpl {
	template<typename... CompTypes>
	struct HasntCompsTester {
		HasntCompsTester(entity_handle entity, World& world) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasntCompsTester<Head, CompTypes...> {
		HasntCompsTester(entity_handle entity, World& world) {
			if (world.hasntComp<Head>(entity)) {
				HasntCompsTester<CompTypes...> recursiveTester(entity, world);
				result = recursiveTester.result;
			}
			else {
				result = false;
			}
		}
		bool result;
	};
}

template<typename... CompTypes>
inline bool World::hasntComps(entity_handle entity) {
	_HasntCompsTesterImpl::HasntCompsTester<CompTypes...> tester(entity, *this);
	return tester.result;
}

// ------------ view implementation ----------------------------------------

template<typename First, typename Second, typename ... CompTypes>
class MultiView {
public:
	MultiView(World& wrld) : world{ wrld }, endID{ static_cast<entity_handle>(world.getEntMemSize()) } {

	}
	template<typename First, typename Second, typename ... CompTypes>
	class iterator {
	public:
		typedef iterator<First, Second, CompTypes...> self_type;
		typedef entity_handle value_type;
		typedef entity_handle& reference;
		typedef entity_handle* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(entity_handle ent, MultiView& vw) : entity{ ent }, view{ vw } {
		}
		inline self_type operator++(int junk) {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			entity++;
			while (entity < view.endID && !(view.world.entities[entity].isValid() && view.world.entities[entity].isSpawned() && view.world.hasComp<First>(entity) && view.world.hasComp<Second>(entity) && view.world.hasComps<CompTypes...>(entity))) entity++;
			assert(entity <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline reference operator*() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return entity;
		}
		inline pointer operator->() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return &entity;
		}
		inline bool operator==(const self_type& rhs) {
			return entity == rhs.entity;
		}
		inline bool operator!=(const self_type& rhs) {
			return entity != rhs.entity;
		}
	private:
		entity_handle entity;
		MultiView& view;
	};
	inline iterator<First, Second, CompTypes...> begin() {
		entity_handle entity = 1;
		while (!(world.hasComp<First>(entity) && world.hasComp<Second>(entity) && world.hasComps<CompTypes...>(entity)) && entity < endID) entity++;
		return iterator<First, Second, CompTypes...>(std::min(entity, endID), *this);
	}
	inline iterator<First, Second, CompTypes...> end() { 
		return iterator<First, Second, CompTypes...>(endID, *this);
	}
private:
	World& world;
	entity_handle endID;
};

template<typename First, typename Second, typename ... CompTypes>
inline MultiView<First, Second, CompTypes...> World::view() {
	return MultiView<First, Second, CompTypes...>(*this);
}

template<typename CompType>
class SingleView {
public:
	SingleView(World& wrld) : world{ wrld }, endID{ static_cast<entity_handle>(world.getEntMemSize()) } {
#ifdef _DEBUG
		componentStorageSizeOnCreate = world.getAll<CompType>().size();
#endif
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator<CompType> self_type;
		typedef entity_handle value_type;
		typedef entity_handle& reference;
		typedef entity_handle* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(entity_handle ent, SingleView& vw) : entity{ ent }, view{ vw } {}
		inline self_type operator++(int junk) {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			entity++;
			while (entity < view.endID && !(view.world.entities[entity].isValid() && view.world.entities[entity].isSpawned() && view.world.hasComp<CompType>(entity))) entity++;
			assert(entity <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline reference operator*() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return entity;
		}
		inline pointer operator->() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return &entity;
		}
		inline bool operator==(const self_type& rhs) {
			return entity == rhs.entity;
		}
		inline bool operator!=(const self_type& rhs) {
			return entity != rhs.entity;
		}
	private:
		entity_handle entity;
		SingleView& view;
	};
	inline iterator<CompType> begin() {
		entity_handle entity = 1;
		while (!world.hasComps<CompType>(entity) && entity < endID) entity++;
		return iterator<CompType>(std::min(entity, endID), *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(endID, *this); }
private:
	World& world;
	entity_handle endID;
#ifdef _DEBUG
	size_t componentStorageSizeOnCreate;
#endif
};

template<typename CompType>
inline SingleView<CompType> World::view() {
	return SingleView<CompType>(*this);
}

// -------- ComponentView implementation -----------------------------------

using Move = Movement;
using Coll = Collider;
using PBody = PhysicsBody;
using TexRef = TextureRef;
class ComponentView {
public:
	ComponentView(World& world, entity_handle entity) : world{ world }, entity{ entity } {}
	template<typename CompType> __forceinline bool has() { return world.hasComp<CompType>(entity); }
	template<typename CompType> __forceinline CompType& add() {
		return world.addComp<CompType>(entity);
	}
	template<typename CompType> __forceinline CompType& add(CompType comp) {
		return world.addComp<CompType>(entity, comp);
	}
	template<typename CompType> __forceinline CompType& get() { return world.getComp<CompType>(entity); }
private:
	World& world;
	entity_handle entity;
};

__forceinline ComponentView World::viewComps(entity_handle entity)
{
	return ComponentView(*this, entity);
}

// ------------------------------------------------------------------------

inline bool World::doesEntExist(entity_handle entity) {
	return (entity < entities.size() ? entities[entity].isValid() : false);
}

inline entity_handle const World::getLastEntity() {
	return latestHandle;
}

__forceinline bool World::areEntsRelated(entity_handle collID, entity_handle otherID) {
	if (hasComp<BaseChild>(collID) && hasComp<BaseChild>(otherID)) {	//same owner no collision check
		if (getEnt(getComp<BaseChild>(collID).parent) == getEnt(getComp<BaseChild>(otherID).parent)) {
			return true;
		}
	}
	else if (hasComp<BaseChild>(collID)) {
		if (getEnt(getComp<BaseChild>(collID).parent) == otherID) {
			return true;
		}
	}
	else if (hasComp<BaseChild>(otherID)) {
		if (getEnt(getComp<BaseChild>(otherID).parent) == collID) {
			return true;
		}
	}
	return false;
}

inline void World::despawn(entity_handle entity)
{
	assert(entities[entity].isValid() && entities[entity].isSpawned());
	if (hasComp<Collider>(entity) && hasntComp<Movement>(entity)) staticEntitiesChanged = true;	// set static changed flag
	entities[entity].setSpawned(false);
}

inline void World::respawn(entity_handle entity)
{
	assert(entities[entity].isValid() && !entities[entity].isSpawned());
	if (hasComp<Collider>(entity) && hasntComp<Movement>(entity)) staticEntitiesChanged = true;	// set static changed flag
	entities[entity].setSpawned(true);
}

inline void World::spawn(entity_handle entity)
{
	assert(doesEntExist(entity));
	respawn(entity);
}

inline bool World::hasID(entity_handle entity)
{
	return entity < handleToId.size() && handleToId[entity].id > 0;
}

inline entity_id World::getID(entity_handle entity) {
	assert(hasID(entity));
	return handleToId[entity];
}

inline bool World::isIDValid(entity_id entityID)
{
	return entityID.id < idToHandle.size() && idToHandle[entityID.id] > 0 && idVersion[entityID.id] == entityID.version;
}

inline entity_handle World::getEnt(entity_id entityID)
{
	assert(isIDValid(entityID));
	return idToHandle[entityID.id];
}

inline void World::setDefragMode(DefragMode mode)
{
	defragMode = mode;
}
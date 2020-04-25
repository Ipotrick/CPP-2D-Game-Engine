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

#define CORE_COMPONENT_COUNT 11
// GameCompWorldInclude API
	#define COMPONENT_INDEX(CompType, Index) template<> struct ComponentIndex<CompType> { static int constexpr index = Index + CORE_COMPONENT_COUNT; };
	#define COMPONENT_STORAGE(CompType, StorageType) ComponentStorage<CompType, StorageType>
#include "GameCompWorldInclude"

#define COMPONENT_INDEX_CORE(CompType, Index) template<> struct ComponentIndex<CompType> { static int constexpr index = Index; };

struct Dummy {};

class Ent {
public:
	Ent(bool valid) : flags{} {
		flags[0] = valid;
		flags[1] = 0;
	}
	__forceinline void setValid(bool valid) {  flags[0] = valid; }
	__forceinline bool isValid() { return flags[0]; }
	__forceinline void setDespawnMark(bool mark) { flags[1] = mark; }
	__forceinline bool isDespawnMarked() { return flags[1]; }

private:
	// flag 0: valid
	// flag 1: despawnMark
	std::bitset<2> flags;
};

template<typename First, typename Second, typename ... CompTypes>
class MultiView;
template<typename CompType>
class SingleView;

class World {
	template<typename First, typename Second, typename ... CompTypes>
	friend class MultiView;
	template<typename CompType>
	friend class SingleView;
public:

	World() : lastID{ 0 }, despawnList{}
	{
		entities.push_back({ false });
	}
	
	/* returnes if entitiy exists or not, O(1) */
	bool doesEntExist(ent_id_t entity);
	/* entity create/destruct utility */
	/* creates blank entity and returns its entity, O(1) */
	ent_id_t createEnt();
	/* enslaves the first ent to the second, ~O(1) */
	void enslaveEntTo(ent_id_t slave, ent_id_t owner, Vec2 relativePos, float relativeRota);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(ent_id_t entity);
	/* returns if an entitiy is related to another entity via a slave/owner relationship */
	bool areEntsRelated(ent_id_t collID, ent_id_t otherID);

	/* world utility */
	/* returnes the entity of the most rescently spawned entity.
		if 0 is returnsed, there are no entities spawned yet.
		Try to not use this function and use the return value of create() instead
		, O(1) */
	ent_id_t const getLastEntID();
	/* returns count of entities, O(1) */
	size_t const getEntCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const getEntMemSize();
	/* sets static entities changed flag */
	void staticsChanged();
	/* returns wheter or not static entities changed */
	bool didStaticsChange();

	/* Component access utility */
	/* returnes reference to a safe virtual container of the given components one can iterate over.
	the iterator also holds the entity entity of the compoenent it points to, O(1) */
	template<typename CompType> auto& getAll();
	/* returnes refference the component data of one entitiy, ~O(1) */
	template<typename CompType> CompType& getComp(ent_id_t entity);
	/* returns bool whether or not the given entity has the component added/registered, ~O(1) */
	template<typename CompType> bool hasComp(ent_id_t entity);
	template<typename ... CompTypes> bool hasComps(ent_id_t entity);
	/* returns bool whether or not the given entity has'nt the component added/registered, ~O(1) */
	template<typename CompType> bool hasntComp(ent_id_t entity);
	template<typename ... CompTypes> bool hasntComps(ent_id_t entity);
	/* adds a new Compoenent to an entity, ~O(1) */
	template<typename CompType> void addComp(ent_id_t entity, CompType data);
	/* adds a new Compoenent to an entity, ~O(1) */
	template<typename CompType> void addComp(ent_id_t entity);
	/* removes a component from the entity */
	template<typename CompType> void remComp(ent_id_t entity);
	/* returnes a View, and iterable object that only iterates over the entities with the given Components */
	template<typename First, typename Second, typename ... CompTypes> MultiView<First, Second, CompTypes...> view();
	template<typename CompType> SingleView<CompType> view();

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
		ComponentStorage<Slave, direct_indexing>,
		ComponentStorage<Composit<4>, hashing>,
		ComponentStorage<CompDataLight, hashing>,
		COMPONENT_SEGMENT
	> componentStorageTuple;
private:
	template<typename CompType>
	struct ComponentIndex {
		static int const index = -1;
	};
	COMPONENT_INDEX_CORE(Base, 0)
	COMPONENT_INDEX_CORE(Movement, 1)
	COMPONENT_INDEX_CORE(Collider, 2)
	COMPONENT_INDEX_CORE(PhysicsBody, 3)
	COMPONENT_INDEX_CORE(LinearEffector , 4)
	COMPONENT_INDEX_CORE(FrictionEffector , 5)
	COMPONENT_INDEX_CORE(Draw , 6)
	COMPONENT_INDEX_CORE(TextureRef , 7)
	COMPONENT_INDEX_CORE(Slave , 8)
	COMPONENT_INDEX_CORE(Composit<4 >, 9)
	COMPONENT_INDEX_CORE(CompDataLight, 1)
	COMPONENT_INDEX_SEGMENT
private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void slaveOwnerDespawn(); // slaves with dead owner get despawned, dead slaves cut their refference of themselfes to the owner
	void deregisterDespawnedEntities();	// CALL BEFORE "executeDespawns"
	void executeDespawns();
	/* resets static entities changed flag*/
	void resetStaticsChangedFlag();
private:
	std::vector<Ent> entities;
	std::queue<ent_id_t> emptySlots;
	ent_id_t lastID;
	std::vector<ent_id_t> despawnList;
	bool staticEntitiesChanged{ true };
};

// ---- Component Accessors implementations --------------------------------

template<typename CompType> __forceinline auto& World::getAll() { 
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple);
} 

template<typename CompType> __forceinline CompType& World::getComp(ent_id_t entity) {
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).get(entity);
} 

template<typename CompType> __forceinline bool World::hasComp(ent_id_t entity) {
	return std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).contains(entity);
} 

template<typename CompType> __forceinline bool World::hasntComp(ent_id_t entity) {
	return !std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).contains(entity);
} 

template<typename CompType> __forceinline void World::addComp(ent_id_t entity, CompType data) {
	std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).insert(entity, data);
} 

template<typename CompType> __forceinline void World::addComp(ent_id_t entity) {
	std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).insert(entity, CompType());
} 

template<typename CompType> __forceinline void World::remComp(ent_id_t entity) {
	std::get<World::ComponentIndex<CompType>::index>(componentStorageTuple).remove(entity);
}

// ---------- hasComps implementation --------------------------------------

namespace _HasCompsTesterImpl {
	template<typename... CompTypes>
	struct HasCompsTester {
		HasCompsTester(ent_id_t entity, World& world) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasCompsTester<Head, CompTypes...> {
		HasCompsTester(ent_id_t entity, World& world) {
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
inline bool World::hasComps(ent_id_t entity) {
	_HasCompsTesterImpl::HasCompsTester<CompTypes...> tester(entity, *this);
	return tester.result;
}

// --------- hasntComps implementation -------------------------------------

namespace _HasntCompsTesterImpl {
	template<typename... CompTypes>
	struct HasntCompsTester {
		HasntCompsTester(ent_id_t entity, World& world) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasntCompsTester<Head, CompTypes...> {
		HasntCompsTester(ent_id_t entity, World& world) {
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
inline bool World::hasntComps(ent_id_t entity) {
	_HasntCompsTesterImpl::HasntCompsTester<CompTypes...> tester(entity, *this);
	return tester.result;
}

// ------------ view implementation ----------------------------------------

template<typename First, typename Second, typename ... CompTypes>
class MultiView {
public:
	MultiView(World& wrld) : world{ wrld }, endID{ static_cast<ent_id_t>(world.getEntMemSize()) } {

	}
	template<typename First, typename Second, typename ... CompTypes>
	class iterator {
	public:
		typedef iterator<First, Second, CompTypes...> self_type;
		typedef ent_id_t value_type;
		typedef ent_id_t& reference;
		typedef ent_id_t* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(ent_id_t ent, MultiView& vw) : entity{ ent }, view{ vw } {
		}
		inline self_type operator++(int junk) {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			entity++;
			while (entity < view.endID && !(view.world.entities[entity].isValid() && view.world.hasComp<First>(entity) && view.world.hasComp<Second>(entity) && view.world.hasComps<CompTypes...>(entity))) entity++;
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
		ent_id_t entity;
		MultiView& view;
	};
	inline iterator<First, Second, CompTypes...> begin() {
		ent_id_t entity = 1;
		while (!(world.hasComp<First>(entity) && world.hasComp<Second>(entity) && world.hasComps<CompTypes...>(entity)) && entity < endID) entity++;
		return iterator<First, Second, CompTypes...>(std::min(entity, endID), *this);
	}
	inline iterator<First, Second, CompTypes...> end() { 
		return iterator<First, Second, CompTypes...>(endID, *this);
	}
private:
	World& world;
	ent_id_t endID;
};

template<typename First, typename Second, typename ... CompTypes>
inline MultiView<First, Second, CompTypes...> World::view() {
	return MultiView<First, Second, CompTypes...>(*this);
}

template<typename CompType>
class SingleView {
public:
	SingleView(World& wrld) : world{ wrld }, endID{ static_cast<ent_id_t>(world.getEntMemSize()) } {
#ifdef _DEBUG
		componentStorageSizeOnCreate = world.getAll<CompType>().size();
#endif
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator<CompType> self_type;
		typedef ent_id_t value_type;
		typedef ent_id_t& reference;
		typedef ent_id_t* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(ent_id_t ent, SingleView& vw) : entity{ ent }, view{ vw } {}
		inline self_type operator++(int junk) {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			assert(view.componentStorageSizeOnCreate == view.world.getAll<CompType>().size());
			entity++;
			while (entity < view.endID && !(view.world.entities[entity].isValid() && view.world.hasComp<CompType>(entity))) entity++;
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
			assert(view.componentStorageSizeOnCreate == view.world.getAll<CompType>().size());
			return entity;
		}
		inline pointer operator->() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			assert(view.componentStorageSizeOnCreate == view.world.getAll<CompType>().size());
			return &entity;
		}
		inline bool operator==(const self_type& rhs) {
			return entity == rhs.entity;
		}
		inline bool operator!=(const self_type& rhs) {
			return entity != rhs.entity;
		}
	private:
		ent_id_t entity;
		SingleView& view;
	};
	inline iterator<CompType> begin() {
		ent_id_t entity = 1;
		while (!world.hasComps<CompType>(entity) && entity < endID) entity++;
		return iterator<CompType>(std::min(entity, endID), *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(endID, *this); }
private:
	World& world;
	ent_id_t endID;
#ifdef _DEBUG
	size_t componentStorageSizeOnCreate;
#endif
};

template<typename CompType>
inline SingleView<CompType> World::view() {
	return SingleView<CompType>(*this);
}

// -------------------------------------------------------------------------

inline bool World::doesEntExist(ent_id_t entity) {
	return (entity < entities.size() ? entities[entity].isValid() : false);
}

inline ent_id_t const World::getLastEntID() {
	return lastID;
}

__forceinline bool World::areEntsRelated(ent_id_t collID, ent_id_t otherID) {
	if (hasComp<Slave>(collID) && hasComp<Slave>(otherID)) {	//same owner no collision check
		if (getComp<Slave>(collID).ownerID == getComp<Slave>(otherID).ownerID) {
			return true;
		}
	}
	else if (hasComp<Slave>(collID)) {
		if (getComp<Slave>(collID).ownerID == otherID) {
			return true;
		}
	}
	else if (hasComp<Slave>(otherID)) {
		if (getComp<Slave>(otherID).ownerID == collID) {
			return true;
		}
	}
	return false;
}
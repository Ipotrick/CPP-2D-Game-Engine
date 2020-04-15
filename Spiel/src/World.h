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
#include "ECS.h"
#include "CoreComponents.h"
#include "GameComponents.h"

#define GENERATE_COMPONENT_ACCESS_FUNCTIONS_INTERN(CompType, CompStorage, storageType) \
template<> __forceinline auto& getAll<CompType>() { return CompStorage; } \
template<> __forceinline CompType& getComp<CompType>(uint32_t id) { return CompStorage.getComponent(id); }\
template<> __forceinline bool hasComp<CompType>(uint32_t id) { return CompStorage.isRegistrated(id); }\
template<> __forceinline void addComp<CompType>(uint32_t id, CompType data) { CompStorage.registrate(id, data); } \
template<> __forceinline void addComp<CompType>(uint32_t id) { CompStorage.registrate(id, CompType()); }

#define GENERATE_COMPONENT_CODE(CompType, StorageType, Num) \
private: ComponentStorage<CompType, StorageType> compStorage ## Num; \
public: GENERATE_COMPONENT_ACCESS_FUNCTIONS_INTERN(CompType, compStorage ## Num, storageType)

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

	World() : lastID{ 0 }, despawnList{}, oldCapacity{0}
	{
		entities.push_back({ false });
	}
	
	/* returnes if entitiy exists or not, O(1) */
	bool doesEntExist(ent_id_t id);
	/* entity create/destruct utility */
	/* creates blank entity and returns its id, O(1) */
	ent_id_t createEnt();
	/* enslaves the first ent to the second, ~O(1) */
	void enslaveEntTo(ent_id_t slave, ent_id_t owner, vec2 relativePos, float relativeRota);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(ent_id_t id);

	/* world utility */
	/* returnes the id of the most rescently spawned entity.
		if 0 is returnsed, there are no entities spawned yet.
		Try to not use this function and use the return value of create() instead
		, O(1) */
	ent_id_t const getLastEntID();
	/* returns count of entities, O(1) */
	size_t const getEntCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const getEntMemSize();
	/* returns wheter or not static entities changed */
	bool didStaticsChange();

	/* Component access utility */
	/* returnes reference to a safe virtual container of the given components one can iterate over. the iterator also holds the entity id of the compoenent it points to, O(1) */
	template<typename CompType> auto& getAll();
	/* returnes refference the component data of one entitiy, ~O(1) */
	template<typename CompType> CompType& getComp(ent_id_t id);
	/* returns bool whether or not the given entity has the component added/registered, ~O(1) */
	template<typename CompType> bool hasComp(ent_id_t id);
	template<typename ... CompTypes> bool hasComps(ent_id_t entity);
	/* registeres a new component under the given id, ~O(1) */
	template<typename CompType> void addComp(ent_id_t id, CompType data);
	/*registeres a new component under the given id, ~O(1) */
	template<typename CompType> void addComp(ent_id_t id);
	/* returnes a View, and iterable object that only iterates over the entities with the given Components */
	template<typename First, typename Second, typename ... CompTypes> MultiView<First, Second, CompTypes...> view();
	template<typename CompType> SingleView<CompType> view();

	void loadMap(std::string);
private:
	GENERATE_COMPONENT_CODE(Base, direct_indexing, 0)
	GENERATE_COMPONENT_CODE(Movement, direct_indexing, 1)
	GENERATE_COMPONENT_CODE(Collider, direct_indexing, 2)
	GENERATE_COMPONENT_CODE(SolidBody, direct_indexing, 3)
	GENERATE_COMPONENT_CODE(Draw, direct_indexing, 4)
	GENERATE_COMPONENT_CODE(TextureRef, direct_indexing, 5)
	GENERATE_COMPONENT_CODE(Slave, direct_indexing, 6)
	GENERATE_COMPONENT_CODE(Composit<4>, hasing, 7)
	GENERATE_COMPONENT_CODE(CompDataLight, hasing, 8)
	GENERATE_COMPONENT_CODE(Health, hasing, 9)
	GENERATE_COMPONENT_CODE(Age, hasing, 10)
	GENERATE_COMPONENT_CODE(Player, hasing, 11)
	GENERATE_COMPONENT_CODE(Bullet, hasing, 12)
	GENERATE_COMPONENT_CODE(Enemy, hasing, 13)
	GENERATE_COMPONENT_CODE(MoveField, hasing, 14)
private:
	//
private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void slaveOwnerDespawn(); // slaves with dead owner get despawned, dead slaves cut their refference of themselfes to the owner
	void deregisterDespawnedEntities();	// CALL BEFORE "executeDespawns"
	void executeDespawns();
private:
	size_t oldCapacity;
	std::vector<Ent> entities;
	std::queue<ent_id_t> emptySlots;
	ent_id_t lastID;
	std::vector<ent_id_t> despawnList;
	bool staticSpawnOrDespawn{ false };
};

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
		while (!world.hasComps<CompType>(entity) && entity < endID) entity++;	// TODO: replace getEntMemSize with component specific sizes
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
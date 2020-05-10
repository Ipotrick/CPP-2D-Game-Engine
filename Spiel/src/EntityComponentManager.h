#pragma once
#include <algorithm>
#include <fstream>

#include <vector>
#include <queue>
#include <deque>
#include <bitset>

#include "robin_hood.h"
#include "json.h"

#include "BaseTypes.h"
#include "RenderTypes.h"
#include "algo.h"

#include "EntityComponentStorage.h"
#include "CoreComponents.h"
#include "CoreCompInclude.h"
#include "GameComponents.h"
#include "GameCompInclude.h"


template<typename First, typename Second, typename ... CompTypes>
class MultiView;
template<typename CompType>
class SingleView;
class ComponentView;

class EntityComponentManager {
public:
	template<typename First, typename Second, typename ... CompTypes>
	friend class MultiView;
	template<typename CompType>
	friend class SingleView;
	class EntityStatus {
	public:
		EntityStatus(bool valid) : flags{} {
			flags[0] = valid;
			flags[1] = false;
			flags[2] = false;
		}
		__forceinline void setValid(bool valid) { flags[0] = valid; }
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
public:
	EntityComponentManager() : latestHandle{ 0 }, defragMode{ DefragMode::NONE }, despawnList{}
	{
		entities.push_back({ false });
		handleToId.push_back(0);
		idToHandle.push_back(0);
	}
	
	/* returnes if entitiy exists or not, O(1) */
	bool exists(entity_handle entity);
	/* entity create/destruct utility */
	/* creates blank entity and returns its entity, O(1) */
	entity_handle create();
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void destroy(entity_handle entity);
	/* immediately makes entity invisible for systems and scripts but the entity stays in memory so it can be respawned */
	void despawn(entity_handle entity);
	/* immediate spawn of entitiy */
	void spawn(entity_handle entity);
	/* delayed(after update call) spawn of an entity */
	void spawnLater(entity_handle entity);


	/* identification */
	/* generates new id for entity or returns existing id */
	entity_id identify(entity_handle entity);
	/* returns handle of the entity, call isIdValid before! */
	entity_handle getEntity(entity_id entityID);
	/* returns true when entity has an id, prefer identify to this */
	bool hasID(entity_handle entity);
	/* if entity has an id it will be returned, call hasID before! Prefer identify to this  */
	entity_id getID(entity_handle entity);
	bool isIdValid(entity_id entityID);
	/* enslaves the first ent to the second, ~O(1) */
	void link(entity_handle slave, entity_handle master, Vec2 relativePos, float relativeRota);
	/* returns if an entitiy is related to another entity via a slave/owner relationship */
	bool areRelated(entity_handle collID, entity_handle otherID);


	/* utility */
	/* returns count of entities, O(1) */
	size_t const entityCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const memorySize();
	/* sets static entities changed flag */
	void setStaticsChanged(bool boolean = true);
	/* returns wheter or not static entities changed */
	bool didStaticsChange();
	/* returnes how fragmented the entities are */
	float fragmentation();
	enum class DefragMode {
		NONE,
		LAZY,
		MODERATE,
		EAGER,
		AGRESSIVE,
		COMPLETE,
		FAST
	};
	/* sets how much at a time and at what fragmentation(%) the manager defragments */
	void setDefragMode(DefragMode mode);

	/* Component access */
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

private:
	std::tuple<CORE_COMPONENT_SEGMENT, GAME_COMPONENT_SEGMENT> componentStorageTuple;
protected:
	void update();
private:
	/* INNER ENGINE FUNCTIONS: */
	void moveEntity(entity_handle start, entity_handle goal);
	entity_handle findBiggestValidHandle();
	void shrink(); // shorten entity array and delete freeHandles at the end of the entity array
	void defragmentEntities();
	void childParentDestroy(); // destroy on parent calls destroy on child
	void parentChildDestroy(); // destroy on child calls destroy on parent
	void deregisterDestroyedEntities();
	void executeDelayedSpawns();
	void executeDestroys();

private:
	std::vector<EntityStatus> entities;
	std::deque<entity_handle> freeHandleQueue;
	entity_handle latestHandle;

	std::vector<entity_handle> idToHandle;
	std::vector<uint32_t> idVersion;
	std::vector<entity_id_type> handleToId;
	std::deque<entity_id_type> freeIdQueue;

	std::vector<entity_handle> despawnList;
	std::vector<entity_handle> spawnLaterList;
	bool staticEntitiesChanged{ true };

	DefragMode defragMode;
};

// ---- Component Accessors implementations --------------------------------

template<typename CompType> __forceinline auto& EntityComponentManager::getAll() {
	static constexpr auto index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<index>(componentStorageTuple);
} 

template<typename CompType> __forceinline CompType& EntityComponentManager::getComp(entity_handle entity) {
	static constexpr auto index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<index>(componentStorageTuple).get(entity);
} 

template<typename CompType> __forceinline bool EntityComponentManager::hasComp(entity_handle entity) {
	static constexpr auto index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<index>(componentStorageTuple).contains(entity);
} 

template<typename CompType> __forceinline bool EntityComponentManager::hasntComp(entity_handle entity) {
	static constexpr auto index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return !std::get<index>(componentStorageTuple).contains(entity);
} 

template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(entity_handle entity, CompType data) {
	static constexpr auto index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<index>(componentStorageTuple).insert(entity, data);
	return std::get<index>(componentStorageTuple)[entity];
} 

template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(entity_handle entity) {
	static constexpr auto index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<index>(componentStorageTuple).insert(entity, CompType());
	return std::get<index>(componentStorageTuple)[entity];
} 

template<typename CompType> __forceinline void EntityComponentManager::remComp(entity_handle entity) {
	static constexpr auto index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<index>(componentStorageTuple).remove(entity);
}

// ---------- hasComps implementation --------------------------------------

namespace _HasCompsTesterImpl {
	template<typename... CompTypes>
	struct HasCompsTester {
		HasCompsTester(entity_handle entity, EntityComponentManager& manager) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasCompsTester<Head, CompTypes...> {
		HasCompsTester(entity_handle entity, EntityComponentManager& manager) {
			if (manager.hasComp<Head>(entity)) {
				HasCompsTester<CompTypes...> recursiveTester(entity, manager);
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
inline bool EntityComponentManager::hasComps(entity_handle entity) {
	_HasCompsTesterImpl::HasCompsTester<CompTypes...> tester(entity, *this);
	return tester.result;
}

// --------- hasntComps implementation -------------------------------------

namespace _HasntCompsTesterImpl {
	template<typename... CompTypes>
	struct HasntCompsTester {
		HasntCompsTester(entity_handle entity, EntityComponentManager& manager) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasntCompsTester<Head, CompTypes...> {
		HasntCompsTester(entity_handle entity, EntityComponentManager& manager) {
			if (manager.hasntComp<Head>(entity)) {
				HasntCompsTester<CompTypes...> recursiveTester(entity, manager);
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
inline bool EntityComponentManager::hasntComps(entity_handle entity) {
	_HasntCompsTesterImpl::HasntCompsTester<CompTypes...> tester(entity, *this);
	return tester.result;
}

// ------------ view implementation ----------------------------------------

template<typename First, typename Second, typename ... CompTypes>
class MultiView {
public:
	MultiView(EntityComponentManager& manager) : manager{ manager }, endID{ static_cast<entity_handle>(manager.memorySize()) } {

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
			assert(view.manager.exists(entity));
			entity++;
			while (entity < view.endID && !(view.manager.entities[entity].isValid() && view.manager.entities[entity].isSpawned() && view.manager.hasComp<First>(entity) && view.manager.hasComp<Second>(entity) && view.manager.hasComps<CompTypes...>(entity))) entity++;
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
			assert(view.manager.exists(entity));
			return entity;
		}
		inline pointer operator->() {
			assert(entity < view.endID);
			assert(view.manager.exists(entity));
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
		while (!(manager.hasComp<First>(entity) && manager.hasComp<Second>(entity) && manager.hasComps<CompTypes...>(entity)) && entity < endID) entity++;
		return iterator<First, Second, CompTypes...>(std::min(entity, endID), *this);
	}
	inline iterator<First, Second, CompTypes...> end() { 
		return iterator<First, Second, CompTypes...>(endID, *this);
	}
private:
	EntityComponentManager& manager;
	entity_handle endID;
};

template<typename First, typename Second, typename ... CompTypes>
inline MultiView<First, Second, CompTypes...> EntityComponentManager::view() {
	return MultiView<First, Second, CompTypes...>(*this);
}

template<typename CompType>
class SingleView {
public:
	SingleView(EntityComponentManager& manager) : manager{ manager }, endID{ static_cast<entity_handle>(manager.memorySize()) } {
#ifdef _DEBUG
		componentStorageSizeOnCreate = manager.getAll<CompType>().size();
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
			assert(view.manager.exists(entity));
			entity++;
			while (entity < view.endID && !(view.manager.entities[entity].isValid() && view.manager.entities[entity].isSpawned() && view.manager.hasComp<CompType>(entity))) entity++;
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
			assert(view.manager.exists(entity));
			return entity;
		}
		inline pointer operator->() {
			assert(entity < view.endID);
			assert(view.manager.exists(entity));
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
		while (!manager.hasComps<CompType>(entity) && entity < endID) entity++;
		return iterator<CompType>(std::min(entity, endID), *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(endID, *this); }
private:
	EntityComponentManager& manager;
	entity_handle endID;
#ifdef _DEBUG
	size_t componentStorageSizeOnCreate;
#endif
};

template<typename CompType>
inline SingleView<CompType> EntityComponentManager::view() {
	return SingleView<CompType>(*this);
}

// -------- ComponentView implementation -----------------------------------

using Move = Movement;
using Coll = Collider;
using PBody = PhysicsBody;
using TexRef = TextureRef;
class ComponentView {
public:
	ComponentView(EntityComponentManager& manager, entity_handle entity) : manager{ manager }, entity{ entity } {}
	template<typename CompType> __forceinline bool has() { return manager.hasComp<CompType>(entity); }
	template<typename CompType> __forceinline CompType& add() {
		return manager.addComp<CompType>(entity);
	}
	template<typename CompType> __forceinline CompType& add(CompType comp) {
		return manager.addComp<CompType>(entity, comp);
	}
	template<typename CompType> __forceinline CompType& get() { return manager.getComp<CompType>(entity); }
private:
	EntityComponentManager& manager;
	entity_handle entity;
};

__forceinline ComponentView EntityComponentManager::viewComps(entity_handle entity)
{
	return ComponentView(*this, entity);
}

// ------------------------------------------------------------------------

inline bool EntityComponentManager::exists(entity_handle entity) {
	return (entity < entities.size() ? entities[entity].isValid() : false);
}

__forceinline bool EntityComponentManager::areRelated(entity_handle collID, entity_handle otherID) {
	if (hasComp<BaseChild>(collID) && hasComp<BaseChild>(otherID)) {	//same owner no collision check
		if (getEntity(getComp<BaseChild>(collID).parent) == getEntity(getComp<BaseChild>(otherID).parent)) {
			return true;
		}
	}
	else if (hasComp<BaseChild>(collID)) {
		if (getEntity(getComp<BaseChild>(collID).parent) == otherID) {
			return true;
		}
	}
	else if (hasComp<BaseChild>(otherID)) {
		if (getEntity(getComp<BaseChild>(otherID).parent) == collID) {
			return true;
		}
	}
	return false;
}

inline void EntityComponentManager::despawn(entity_handle entity)
{
	assert(entities[entity].isValid() && entities[entity].isSpawned());
	if (hasComp<Collider>(entity) && hasntComp<Movement>(entity)) staticEntitiesChanged = true;	// set static changed flag
	entities[entity].setSpawned(false);
}

inline void EntityComponentManager::spawn(entity_handle entity)
{
	assert(entities[entity].isValid() && !entities[entity].isSpawned());
	if (hasComp<Collider>(entity) && hasntComp<Movement>(entity)) staticEntitiesChanged = true;	// set static changed flag
	entities[entity].setSpawned(true);
}

inline bool EntityComponentManager::hasID(entity_handle entity)
{
	return entity < handleToId.size() && handleToId[entity] > 0;
}

inline entity_id EntityComponentManager::getID(entity_handle entity) {
	assert(hasID(entity));
	return entity_id(handleToId[entity], idVersion[handleToId[entity]]);
}

inline bool EntityComponentManager::isIdValid(entity_id entityID)
{
	return entityID.id < idToHandle.size() && idToHandle[entityID.id] > 0 && idVersion[entityID.id] == entityID.version;
}

inline entity_handle EntityComponentManager::getEntity(entity_id entityID)
{
	assert(isIdValid(entityID));
	return idToHandle[entityID.id];
}

inline void EntityComponentManager::setDefragMode(DefragMode mode)
{
	defragMode = mode;
}
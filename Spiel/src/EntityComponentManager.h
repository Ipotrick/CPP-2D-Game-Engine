#pragma once
#include <algorithm>
#include <fstream>

#include <vector>
#include <queue>
#include <deque>
#include <bitset>
#include <tuple>
#include <type_traits>

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
class MultiViewIDX;
template<typename CompType>
class SingleViewIDX;
template<typename First, typename Second, typename ... CompTypes>
class MultiView;
template<typename CompType>
class SingleView;
class ComponentView;

class EntityComponentManager {
public:
	template<typename First, typename Second, typename ... CompTypes>
	friend class MultiViewIDX;
	template<typename CompType>
	friend class SingleViewIDX;
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
	EntityComponentManager() : latestIndex{ 0 }, despawnList{}
	{
		entityStorageInfo.push_back({ false });
		indexToId.push_back(0);
		idToIndex.push_back(0);
		idVersion.push_back(0);
	}
	
	/* returnes if entitiy exists or not, O(1) */
	bool exists(entity_index_type index);
	bool exists(entity_id id);
	/* index create/destruct utility */
	/* creates blank index and returns its index, O(1) */
	entity_index_type index_create();
	entity_id create();
	/* marks index for deletion, entities are deleted after each update, O(1) */
	void destroy(entity_index_type index);
	void destroy(entity_id id);
	/* immediately makes index invisible for systems and scripts but the index stays in memory so it can be respawned */
	void despawn(entity_index_type index);
	void despawn(entity_id id);
	/* immediate spawn of entitiy */
	void spawn(entity_index_type index);
	void spawn(entity_id id);
	/* delayed(after update call) spawn of an index */
	void spawnLater(entity_index_type index);
	void spawnLater(entity_id id);


	/* identification */
	/* generates new id for index or returns existing id */
	entity_id identify(entity_index_type index);
	/* returns handle of the index, call isIdValid before! */
	entity_index_type getIndex(entity_id entityID);
	/* returns true when index has an id, prefer identify to this */
	bool hasID(entity_index_type index);
	/* if index has an id it will be returned, call hasID before! Prefer identify to this  */
	entity_id getID(entity_index_type index);
	bool isIdValid(entity_id entityID);
	/* enslaves the first ent to the second, ~O(1) */
	void link(entity_index_type slave, entity_index_type master, Vec2 relativePos, float relativeRota);
	/* returns if an entitiy is related to another index via a slave/owner relationship */
	bool areRelated(entity_index_type collID, entity_index_type otherID);


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

	/* Component access */
	/* returnes reference to a safe virtual container of the given components one can iterate over.
	the iterator also holds the index index of the compoenent it points to, O(1) */
	template<typename CompType> auto& getAll();
	/* returnes refference the component data of one entitiy, ~O(1) */
	template<typename CompType> CompType& getComp(entity_index_type index);
	template<typename CompType> CompType& getComp(entity_id id);
	template<typename ... CompTypes> auto getCompPtrs(entity_index_type index)-> const std::tuple<CompTypes...>;
	template<typename ... CompTypes> auto getCompPtrs(entity_id id)-> const std::tuple<CompTypes...>;
	/* returns bool whether or not the given index has the component added/registered, ~O(1) */
	template<typename CompType> bool hasComp(entity_index_type index);
	template<typename CompType> bool hasComp(entity_id id);
	template<typename ... CompTypes> bool hasComps(entity_index_type index);
	template<typename ... CompTypes> bool hasComps(entity_id id);
	/* returns bool whether or not the given index has'nt the component added/registered, ~O(1) */
	template<typename CompType> bool hasntComp(entity_index_type index);
	template<typename CompType> bool hasntComp(entity_id id);
	template<typename ... CompTypes> bool hasntComps(entity_index_type index);
	template<typename ... CompTypes> bool hasntComps(entity_id id);
	/* adds a new Compoenent to an index, ~O(1) */
	template<typename CompType> CompType& addComp(entity_index_type index, CompType data);
	template<typename CompType> CompType& addComp(entity_id id, CompType data);
	/* adds a new Compoenent to an index, ~O(1) */
	template<typename CompType> CompType& addComp(entity_index_type index);
	template<typename CompType> CompType& addComp(entity_id id);
	/* removes a component from the index */
	template<typename CompType> void remComp(entity_index_type index);
	template<typename CompType> void remComp(entity_id id);
	/* returnes a View, and iterable object that only iterates over the entities with the given Components, ignored despawned entities */
	template<typename First, typename Second, typename ... CompTypes> MultiViewIDX<First, Second, CompTypes...> index_view();
	template<typename CompType> SingleViewIDX<CompType> index_view();
	template<typename First, typename Second, typename ... CompTypes> MultiView<First, Second, CompTypes...> view();
	template<typename CompType> SingleView<CompType> view();
	ComponentView viewComps(entity_index_type index);
	ComponentView viewComps(entity_id id);

	// Meta utility:
	void flushLaterActions();
	enum class DefragMode {
		NONE,
		LAZY,
		MODERATE,
		EAGER,
		AGRESSIVE,
		COMPLETE,
		FAST
	};
	void defragment(DefragMode mode);
private:
	std::tuple<CORE_COMPONENT_SEGMENT, GAME_COMPONENT_SEGMENT> componentStorageTuple;
private:
	/* INNER ENGINE FUNCTIONS: */
	void moveEntity(entity_index_type start, entity_index_type goal);
	entity_index_type findBiggestValidHandle();
	void shrink(); // shorten index array and delete freeHandles at the end of the index array
	void childParentDestroy(); // destroy on parent calls destroy on child
	void parentChildDestroy(); // destroy on child calls destroy on parent
	void deregisterDestroyedEntities();
	void executeDelayedSpawns();
	void executeDestroys();

private:
	std::vector<EntityStatus> entityStorageInfo;
	std::deque<entity_index_type> freeIndexQueue;
	entity_index_type latestIndex;

	std::vector<entity_index_type> idToIndex;
	std::vector<uint32_t> idVersion;
	std::vector<entity_id_type> indexToId;
	std::deque<entity_id_type> freeIdQueue;

	std::vector<entity_index_type> despawnList;
	std::vector<entity_index_type> spawnLaterList;
	bool staticEntitiesChanged{ true };
};

// ---- Component Accessors implementations --------------------------------

template<typename CompType> __forceinline auto& EntityComponentManager::getAll() {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<tuple_index>(componentStorageTuple);
} 

template<typename CompType> __forceinline CompType& EntityComponentManager::getComp(entity_index_type index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<tuple_index>(componentStorageTuple).get(index);
}
template<typename CompType>
inline CompType& EntityComponentManager::getComp(entity_id id)
{
	return getComp<CompType>(idToIndex[id.id]);
}

template<typename ... CompTypes> auto EntityComponentManager::getCompPtrs(entity_index_type index) -> const std::tuple<CompTypes...> {
	std::tuple<CompTypes...> res;
	for_each(res, [&](auto ref) {
		ref = &(getComp<std::remove_pointer<decltype(ref)>::type>(index));
		}, CompTypes...);
	return res;
}

template<typename ... CompTypes> auto EntityComponentManager::getCompPtrs(entity_id id) -> const std::tuple<CompTypes...> {
	std::tuple<CompTypes...> res;
	for_each(res, [&](auto& ref) {
		ref = &(getComp<std::remove_pointer<std::remove_reference<decltype(ref)>::type>::type>(id));
		});
	return res;
}

template<typename CompType> __forceinline bool EntityComponentManager::hasComp(entity_index_type index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<tuple_index>(componentStorageTuple).contains(index);
}
template<typename CompType>
inline bool EntityComponentManager::hasComp(entity_id id)
{
	return hasComp<CompType>(idToIndex[id.id]);
}


template<typename CompType> __forceinline bool EntityComponentManager::hasntComp(entity_index_type index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return !std::get<tuple_index>(componentStorageTuple).contains(index);
}
template<typename CompType>
inline bool EntityComponentManager::hasntComp(entity_id id)
{
	return hasntComp<CompType>(idToIndex[id.id]);
}


template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(entity_index_type index, CompType data) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).insert(index, data);
	return std::get<tuple_index>(componentStorageTuple)[index];
}
template<typename CompType>
inline CompType& EntityComponentManager::addComp(entity_id id, CompType data)
{
	return addComp<CompType>(idToIndex[id.id], data);
}


template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(entity_index_type index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).insert(index, CompType());
	return std::get<tuple_index>(componentStorageTuple)[index];
}
template<typename CompType>
inline CompType& EntityComponentManager::addComp(entity_id id)
{
	return addComp<CompType>(idToIndex[id.id]);
}


template<typename CompType> __forceinline void EntityComponentManager::remComp(entity_index_type index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).remove(index);
}

template<typename CompType>
inline void EntityComponentManager::remComp(entity_id id)
{
	remComp<CompType>(idToIndex[id.id]);
}

// ---------- hasComps implementation --------------------------------------

namespace _HasCompsTesterImpl {
	template<typename... CompTypes>
	struct HasCompsTester {
		HasCompsTester(entity_index_type index, EntityComponentManager& manager) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasCompsTester<Head, CompTypes...> {
		HasCompsTester(entity_index_type index, EntityComponentManager& manager) {
			result = manager.hasComp<Head>(index) & HasCompsTester<CompTypes...>(index, manager).result;
		}
		bool result;
	};
}

template<typename... CompTypes>
inline bool EntityComponentManager::hasComps(entity_index_type index) {
	_HasCompsTesterImpl::HasCompsTester<CompTypes...> tester(index, *this);
	return tester.result;
}

template<typename ...CompTypes>
inline bool EntityComponentManager::hasComps(entity_id id)
{
	return hasComps<CompTypes...>(idToIndex[id.id]);
}

// --------- hasntComps implementation -------------------------------------

namespace _HasntCompsTesterImpl {
	template<typename... CompTypes>
	struct HasntCompsTester {
		HasntCompsTester(entity_index_type index, EntityComponentManager& manager) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasntCompsTester<Head, CompTypes...> {
		HasntCompsTester(entity_index_type index, EntityComponentManager& manager) {
			if (manager.hasntComp<Head>(index)) {
				HasntCompsTester<CompTypes...> recursiveTester(index, manager);
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
inline bool EntityComponentManager::hasntComps(entity_index_type index) {
	_HasntCompsTesterImpl::HasntCompsTester<CompTypes...> tester(index, *this);
	return tester.result;
}

template<typename ...CompTypes>
inline bool EntityComponentManager::hasntComps(entity_id id)
{
	return hasntComps<CompTypes...>(idToIndex[id.id]);
}

// ------------ view implementation ----------------------------------------
// ----------------- multi view --------------------------------------------

template<typename First, typename Second, typename ... CompTypes>
class MultiViewIDX {
public:
	MultiViewIDX(EntityComponentManager& manager) : manager{ manager }, endID{ static_cast<entity_index_type>(manager.memorySize()) } {

	}
	template<typename First, typename Second, typename ... CompTypes>
	class iterator {
	public:
		typedef iterator<First, Second, CompTypes...> self_type;
		typedef entity_index_type value_type;
		typedef entity_index_type& reference;
		typedef entity_index_type* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(entity_index_type ent, MultiViewIDX& vw) : index{ ent }, view{ vw } {
		}
		inline self_type operator++(int junk) {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			index++;
			while (index < view.endID && !(view.manager.entityStorageInfo[index].isValid() && view.manager.entityStorageInfo[index].isSpawned() && view.manager.hasComp<First>(index) && view.manager.hasComp<Second>(index) && view.manager.hasComps<CompTypes...>(index))) index++;
			assert(index <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline value_type operator*() {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			return index;
		}
		inline bool operator==(const self_type& rhs) {
			return index == rhs.index;
		}
		inline bool operator!=(const self_type& rhs) {
			return index != rhs.index;
		}
	private:
		entity_index_type index;
		MultiViewIDX& view;
	};
	inline iterator<First, Second, CompTypes...> begin() {
		entity_index_type index = 1;
		while (!(manager.hasComp<First>(index) && manager.hasComp<Second>(index) && manager.hasComps<CompTypes...>(index)) && index < endID) index++;
		return iterator<First, Second, CompTypes...>(std::min(index, endID), *this);
	}
	inline iterator<First, Second, CompTypes...> end() { 
		return iterator<First, Second, CompTypes...>(endID, *this);
	}
private:
	EntityComponentManager& manager;
	entity_index_type endID;
};

template<typename First, typename Second, typename ... CompTypes>
inline MultiViewIDX<First, Second, CompTypes...> EntityComponentManager::index_view() {
	return MultiViewIDX<First, Second, CompTypes...>(*this);
}

// ----------- multi view idx -------------------

template<typename First, typename Second, typename ... CompTypes>
class MultiView {
public:
	MultiView(EntityComponentManager& manager) : manager{ manager }, endID{ static_cast<entity_index_type>(manager.memorySize()) } {

	}
	template<typename First, typename Second, typename ... CompTypes>
	class iterator {
	public:
		typedef iterator<First, Second, CompTypes...> self_type;
		typedef entity_id value_type;
		typedef entity_id& reference;
		typedef entity_id* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(entity_index_type ent, MultiView& vw) : index{ ent }, view{ vw } {
		}
		inline self_type operator++(int junk) {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			index++;
			while (index < view.endID && !(view.manager.entityStorageInfo[index].isValid() && view.manager.entityStorageInfo[index].isSpawned() && view.manager.hasComp<First>(index) && view.manager.hasComp<Second>(index) && view.manager.hasComps<CompTypes...>(index))) index++;
			assert(index <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline value_type operator*() {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			auto id = view.manager.indexToId[index];
			return entity_id(id, view.manager.idVersion[id]);
		}
		inline bool operator==(const self_type& rhs) {
			return index == rhs.index;
		}
		inline bool operator!=(const self_type& rhs) {
			return index != rhs.index;
		}
	private:
		entity_index_type index;
		MultiView& view;
	};
	inline iterator<First, Second, CompTypes...> begin() {
		entity_index_type index = 1;
		while (!(manager.hasComp<First>(index) && manager.hasComp<Second>(index) && manager.hasComps<CompTypes...>(index)) && index < endID) index++;
		return iterator<First, Second, CompTypes...>(std::min(index, endID), *this);
	}
	inline iterator<First, Second, CompTypes...> end() {
		return iterator<First, Second, CompTypes...>(endID, *this);
	}
private:
	EntityComponentManager& manager;
	entity_index_type endID;
};

template<typename First, typename Second, typename ... CompTypes>
inline MultiView<First, Second, CompTypes...> EntityComponentManager::view() {
	return MultiView<First, Second, CompTypes...>(*this);
}
// ---------- single view idx -------------------

template<typename CompType>
class SingleViewIDX {
public:
	SingleViewIDX(EntityComponentManager& manager) : manager{ manager }, endID{ static_cast<entity_index_type>(manager.memorySize()) } {
#ifdef _DEBUG
		componentStorageSizeOnCreate = manager.getAll<CompType>().size();
#endif
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator<CompType> self_type;
		typedef entity_index_type value_type;
		typedef entity_index_type& reference;
		typedef entity_index_type* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(entity_index_type ent, SingleViewIDX& vw) : index{ ent }, view{ vw } {}
		inline self_type operator++(int junk) {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			index++;
			while (index < view.endID && !(view.manager.entityStorageInfo[index].isValid() && view.manager.entityStorageInfo[index].isSpawned() && view.manager.hasComp<CompType>(index))) index++;
			assert(index <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline value_type operator*() {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			return index;
		}
		inline bool operator==(const self_type& rhs) {
			return index == rhs.index;
		}
		inline bool operator!=(const self_type& rhs) {
			return index != rhs.index;
		}
	private:
		entity_index_type index;
		SingleViewIDX& view;
	};
	inline iterator<CompType> begin() {
		entity_index_type index = 1;
		while (!manager.hasComps<CompType>(index) && index < endID) index++;
		return iterator<CompType>(std::min(index, endID), *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(endID, *this); }
private:
	EntityComponentManager& manager;
	entity_index_type endID;
#ifdef _DEBUG
	size_t componentStorageSizeOnCreate;
#endif
};

template<typename CompType>
inline SingleViewIDX<CompType> EntityComponentManager::index_view() {
	return SingleViewIDX<CompType>(*this);
}


// ---------- single view -------------------

template<typename CompType>
class SingleView {
public:
	SingleView(EntityComponentManager& manager) : manager{ manager }, endID{ static_cast<entity_index_type>(manager.memorySize()) } {
#ifdef _DEBUG
		componentStorageSizeOnCreate = manager.getAll<CompType>().size();
#endif
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator<CompType> self_type;
		typedef entity_id value_type;
		typedef entity_id& reference;
		typedef entity_id* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(entity_index_type ent, SingleView& vw) : index{ ent }, view{ vw } {}
		inline self_type operator++(int junk) {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			index++;
			while (index < view.endID && !(view.manager.entityStorageInfo[index].isValid() && view.manager.entityStorageInfo[index].isSpawned() && view.manager.hasComp<CompType>(index))) index++;
			assert(index <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline value_type operator*() {
			assert(index < view.endID);
			assert(view.manager.exists(index));
			auto id = view.manager.indexToId[index];
			return entity_id(id, view.manager.idVersion[id]);
		}
		inline bool operator==(const self_type& rhs) {
			return index == rhs.index;
		}
		inline bool operator!=(const self_type& rhs) {
			return index != rhs.index;
		}
	private:
		entity_index_type index;
		SingleView& view;
	};
	inline iterator<CompType> begin() {
		entity_index_type index = 1;
		while (!manager.hasComps<CompType>(index) && index < endID) index++;
		return iterator<CompType>(std::min(index, endID), *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(endID, *this); }
private:
	EntityComponentManager& manager;
	entity_index_type endID;
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
	ComponentView(EntityComponentManager& manager, entity_index_type index) : manager{ manager }, index{ index } {}
	template<typename CompType> __forceinline bool has() { return manager.hasComp<CompType>(index); }
	template<typename CompType> __forceinline CompType& add() {
		return manager.addComp<CompType>(index);
	}
	template<typename CompType> __forceinline CompType& add(CompType comp) {
		return manager.addComp<CompType>(index, comp);
	}
	template<typename CompType> __forceinline CompType& get() { return manager.getComp<CompType>(index); }
private:
	EntityComponentManager& manager;
	entity_index_type index;
};

__forceinline ComponentView EntityComponentManager::viewComps(entity_index_type index)
{
	return ComponentView(*this, index);
}

inline ComponentView EntityComponentManager::viewComps(entity_id id)
{
	return ComponentView(*this, idToIndex[id.id]);
}

// ------------------------------------------------------------------------

inline bool EntityComponentManager::exists(entity_index_type index) {
	return (index < entityStorageInfo.size() ? entityStorageInfo[index].isValid() : false);
}

inline bool EntityComponentManager::exists(entity_id id)
{
	return exists(idToIndex[id.id]);
}

__forceinline bool EntityComponentManager::areRelated(entity_index_type collID, entity_index_type otherID) {
	if (hasComp<BaseChild>(collID) && hasComp<BaseChild>(otherID)) {	//same owner no collision check
		if (getIndex(getComp<BaseChild>(collID).parent) == getIndex(getComp<BaseChild>(otherID).parent)) {
			return true;
		}
	}
	else if (hasComp<BaseChild>(collID)) {
		if (getIndex(getComp<BaseChild>(collID).parent) == otherID) {
			return true;
		}
	}
	else if (hasComp<BaseChild>(otherID)) {
		if (getIndex(getComp<BaseChild>(otherID).parent) == collID) {
			return true;
		}
	}
	return false;
}

inline void EntityComponentManager::despawn(entity_index_type index)
{
	assert(entityStorageInfo[index].isValid() && entityStorageInfo[index].isSpawned());
	if (hasComp<Collider>(index) && hasntComp<Movement>(index)) staticEntitiesChanged = true;	// set static changed flag
	entityStorageInfo[index].setSpawned(false);
}

inline void EntityComponentManager::despawn(entity_id id)
{
	despawn(idToIndex[id.id]);
}

inline void EntityComponentManager::spawn(entity_index_type index)
{
	assert(entityStorageInfo[index].isValid() && !entityStorageInfo[index].isSpawned());
	if (hasComp<Collider>(index) && hasntComp<Movement>(index)) staticEntitiesChanged = true;	// set static changed flag
	entityStorageInfo[index].setSpawned(true);
}

inline void EntityComponentManager::spawn(entity_id id)
{
	spawn(idToIndex[id.id]);
}

inline bool EntityComponentManager::hasID(entity_index_type index)
{
	return index < indexToId.size() && indexToId[index] > 0;
}

inline entity_id EntityComponentManager::getID(entity_index_type index) {
	assert(hasID(index));
	return entity_id(indexToId[index], idVersion[indexToId[index]]);
}

inline bool EntityComponentManager::isIdValid(entity_id entityID)
{
	return entityID.id < idToIndex.size() && idToIndex[entityID.id] > 0 && idVersion[entityID.id] == entityID.version;
}

inline entity_index_type EntityComponentManager::getIndex(entity_id entityID)
{
	assert(isIdValid(entityID));
	return idToIndex[entityID.id];
}
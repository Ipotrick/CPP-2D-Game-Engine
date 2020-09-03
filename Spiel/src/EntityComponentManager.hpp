#pragma once
#include <algorithm>
#include <fstream>

#include <vector>
#include <queue>
#include <deque>
#include <bitset>
#include <tuple>
#include <type_traits>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp >

#include "robin_hood.h"
#include "json.h"

#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "algo.hpp"

#include "EntityComponentStorage.hpp"
#include "CoreComponents.hpp"
#include "CoreCompInclude.hpp"
#include "GameComponents.hpp"
#include "GameCompInclude.hpp"


template<typename First, typename Second, typename ... CompTypes>
class MultiView;
template<typename Comp>
class SingleView;
class ComponentView;

class EntityComponentManager {
protected:
	friend class Game;
	template<typename First, typename Second, typename ... CompTypes>
	friend class MultiView;
	template<typename Comp>
	friend class SingleView;

	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int version) const
	{
		//TODO IMPLEMENT MAXIMUM DEFRAGMENTATION BEFORE SAVE
		ar << entityStorageInfo;
		ar << freeIdQueue;
		ar << latestIndex;
		ar << idToIndex;
		ar << idVersion;
		ar << indexToId;
		ar << freeIdQueue;
		ar << despawnList;
		ar << spawnLaterList;
		ar << staticEntitiesChanged;

		for_each(componentStorageTuple, [&](auto& componentStorage) {
			ar << componentStorage.dump();
			});
	}

	template<class Archive>
	void load(Archive& ar, const unsigned int version)
	{
		ar >> entityStorageInfo;
		ar >> freeIdQueue;
		ar >> latestIndex;
		ar >> idToIndex;
		ar >> idVersion;
		ar >> indexToId;
		ar >> freeIdQueue;
		ar >> despawnList;
		ar >> spawnLaterList;
		ar >> staticEntitiesChanged;

		for_each(componentStorageTuple, [&](auto& componentStorage) {
			componentStorage.updateMaxEntNum(entityStorageInfo.size());
			decltype(componentStorage.dump()) dump;
			ar >> dump;
			for (auto [ent, comp] : dump) {
				componentStorage.insert(ent, comp);
			}
			});
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	class EntityStatus {
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int file_version)
		{
			ar& flags;
		}
	public:
		EntityStatus() = default;
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
	bool exists(Entity index);
	bool exists(EntityId id);
	/* index create/destruct utility */
	/* creates blank index and returns its index, O(1) */
	Entity index_create();
	EntityId id_create();
	/* marks index for deletion, entities are deleted after each update, O(1) */
	void destroy(Entity index);
	void destroy(EntityId id);
	/* immediately makes index invisible for systems and scripts but the index stays in memory so it can be respawned */
	void despawn(Entity index);
	void despawn(EntityId id);
	/* immediate spawn of entitiy */
	void spawn(Entity index);
	void spawn(EntityId id);
	/* delayed(after update call) spawn of an index */
	void spawnLater(Entity index);
	void spawnLater(EntityId id);


	/* identification */
	/* generates new id for index or returns existing id */
	EntityId makeId(Entity index);
	/* returns handle of the index, call isIdValid before! */
	Entity getIndex(EntityId entityId);
	/* returns true when index has an id, prefer identify to this */
	bool hasID(Entity index);
	/* if index has an id it will be returned, call hasID before! Prefer identify to this  */
	EntityId getID(Entity index);
	/* if the version of the given Id deviates from the one that is in the regestry, the given id is invalid */
	bool isIdValid(EntityId entityId);
	/* returns if entity is currently in the world/spawned */
	bool isSpawned(Entity ent);
	bool isSpawned(EntityId ent);

	/* utility */
	/* returns count of entities, O(1) */
	size_t const entityCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const maxEntityIndex();
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
	template<typename CompType> CompType& getComp(Entity index);
	template<typename CompType> CompType& getComp(EntityId id);
	template<typename ... CompTypes> auto getCompPtrs(Entity index)-> const std::tuple<CompTypes...>;
	template<typename ... CompTypes> auto getCompPtrs(EntityId id)-> const std::tuple<CompTypes...>;
	/* returns bool whether or not the given index has the component added/registered, ~O(1) */
	template<typename CompType> bool hasComp(Entity index);
	template<typename CompType> bool hasComp(EntityId id);
	template<typename ... CompTypes> bool hasComps(Entity index);
	template<typename ... CompTypes> bool hasComps(EntityId id);
	/* returns bool whether or not the given index has'nt the component added/registered, ~O(1) */
	template<typename CompType> bool hasntComp(Entity index);
	template<typename CompType> bool hasntComp(EntityId id);
	template<typename ... CompTypes> bool hasntComps(Entity index);
	template<typename ... CompTypes> bool hasntComps(EntityId id);
	/* adds a new Compoenent to an index, ~O(1) */
	template<typename CompType> CompType& addComp(Entity index, CompType data);
	template<typename CompType> CompType& addComp(EntityId id, CompType data);
	/* adds a new Compoenent to an index, ~O(1) */
	template<typename CompType> CompType& addComp(Entity index);
	template<typename CompType> CompType& addComp(EntityId id);
	/* removes a component from the index */
	template<typename CompType> void remComp(Entity index);
	template<typename CompType> void remComp(EntityId id);
	/* returnes a View, and iterable object that only iterates over the entities with the given Components, ignored despawned entities */
	template<typename First, typename Second, typename ... CompTypes> 
	[[nodiscard]] auto entity_view();
	template<typename CompType> 
	[[nodiscard]] auto entity_view();
	[[nodiscard]] ComponentView viewComps(Entity index);
	[[nodiscard]] ComponentView viewComps(EntityId id);

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
	void moveEntity(Entity start, Entity goal);
	Entity findBiggestValidHandle();
	void shrink(); // shorten index array and delete freeHandles at the end of the index array
	void deregisterDestroyedEntities();
	void executeDelayedSpawns();
	void executeDestroys();

private:
	std::vector<EntityStatus> entityStorageInfo;
	std::deque<Entity> freeIndexQueue;
	Entity latestIndex;

	std::vector<Entity> idToIndex;
	std::vector<entity_id_t> idVersion;
	std::vector<entity_id_t> indexToId;
	std::deque<entity_id_t> freeIdQueue;

	std::vector<Entity> despawnList;
	std::vector<Entity> spawnLaterList;
	bool staticEntitiesChanged{ true };
};

// ---- Component Accessors implementations --------------------------------

template<typename CompType> __forceinline auto& EntityComponentManager::getAll() {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<tuple_index>(componentStorageTuple);
} 

template<typename CompType> __forceinline CompType& EntityComponentManager::getComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<tuple_index>(componentStorageTuple).get(index);
}
template<typename CompType>
inline CompType& EntityComponentManager::getComp(EntityId id)
{
	return getComp<CompType>(idToIndex[id.id]);
}

template<typename ... CompTypes> auto EntityComponentManager::getCompPtrs(Entity index) -> const std::tuple<CompTypes...> {
	std::tuple<CompTypes...> res;
	for_each(res, [&](auto ref) {
		ref = &(getComp<std::remove_pointer<decltype(ref)>::type>(index));
		}, CompTypes...);
	return res;
}

template<typename ... CompTypes> auto EntityComponentManager::getCompPtrs(EntityId id) -> const std::tuple<CompTypes...> {
	std::tuple<CompTypes...> res;
	for_each(res, [&](auto& ref) {
		ref = &(getComp<std::remove_pointer<std::remove_reference<decltype(ref)>::type>::type>(id));
		});
	return res;
}

template<typename CompType> __forceinline bool EntityComponentManager::hasComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return std::get<tuple_index>(componentStorageTuple).contains(index);
}
template<typename CompType>
inline bool EntityComponentManager::hasComp(EntityId id)
{
	return hasComp<CompType>(idToIndex[id.id]);
}

template<typename CompType> __forceinline bool EntityComponentManager::hasntComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return !std::get<tuple_index>(componentStorageTuple).contains(index);
}
template<typename CompType>
inline bool EntityComponentManager::hasntComp(EntityId id)
{
	return hasntComp<CompType>(idToIndex[id.id]);
}

template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(Entity index, CompType data) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).insert(index, data);
	return std::get<tuple_index>(componentStorageTuple)[index];
}
template<typename CompType>
inline CompType& EntityComponentManager::addComp(EntityId id, CompType data)
{
	return addComp<CompType>(idToIndex[id.id], data);
}

template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).insert(index, CompType());
	return std::get<tuple_index>(componentStorageTuple)[index];
}
template<typename CompType>
inline CompType& EntityComponentManager::addComp(EntityId id)
{
	return addComp<CompType>(idToIndex[id.id]);
}


template<typename CompType> __forceinline void EntityComponentManager::remComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).remove(index);
}

template<typename CompType>
inline void EntityComponentManager::remComp(EntityId id)
{
	remComp<CompType>(idToIndex[id.id]);
}

// ---------- hasComps implementation --------------------------------------

namespace _HasCompsTesterImpl {
	template<typename... CompTypes>
	struct HasCompsTester {
		HasCompsTester(Entity index, EntityComponentManager& manager) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasCompsTester<Head, CompTypes...> {
		HasCompsTester(Entity index, EntityComponentManager& manager) {
			result = manager.hasComp<Head>(index) & HasCompsTester<CompTypes...>(index, manager).result;
		}
		bool result;
	};
}

template<typename... CompTypes>
inline bool EntityComponentManager::hasComps(Entity index) {
	_HasCompsTesterImpl::HasCompsTester<CompTypes...> tester(index, *this);
	return tester.result;
}

template<typename ...CompTypes>
inline bool EntityComponentManager::hasComps(EntityId id)
{
	return hasComps<CompTypes...>(idToIndex[id.id]);
}

// --------- hasntComps implementation -------------------------------------

namespace _HasntCompsTesterImpl {
	template<typename... CompTypes>
	struct HasntCompsTester {
		HasntCompsTester(Entity index, EntityComponentManager& manager) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasntCompsTester<Head, CompTypes...> {
		HasntCompsTester(Entity index, EntityComponentManager& manager) {
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
inline bool EntityComponentManager::hasntComps(Entity index) {
	_HasntCompsTesterImpl::HasntCompsTester<CompTypes...> tester(index, *this);
	return tester.result;
}

template<typename ...CompTypes>
inline bool EntityComponentManager::hasntComps(EntityId id)
{
	return hasntComps<CompTypes...>(idToIndex[id.id]);
}

// ------------ view implementation ----------------------------------------

template<typename CompStore>
class SingleView {
public:
	SingleView(EntityComponentManager& manager, CompStore& compStore)
		: manager{ manager }, compStore{ compStore }, iterEnd{ compStore.end() }
	{ }
	template<typename MainIterT>
	class iterator {
	public:
		typedef iterator<MainIterT> self_type;
		typedef Entity value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(const MainIterT iter, SingleView& vw) 
			: iter{ iter }, view{ vw } 
		{ }
		inline self_type operator++(int junk)
		{
			do {
				++iter;
			} while (iter != view.iterEnd && !view.manager.isSpawned(*iter));
			return *this;
		}
		inline self_type operator++()
		{
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline value_type operator*()
		{
			return *iter;
		}
		inline bool operator==(const self_type& rhs)
		{
			return iter == rhs.iter;
		}
		inline bool operator!=(const self_type& rhs)
		{
			return iter != rhs.iter;
		}
	private:
		MainIterT iter;
		SingleView& view;
	};
	inline auto begin()
	{
		auto iter = compStore.begin();
		return iterator<decltype(compStore.end())>(iter, *this);
	}
	inline auto end()
	{
		return iterator<decltype(compStore.end())>(iterEnd, *this);
	}
private:
	EntityComponentManager& manager;
	CompStore& compStore; 
	const decltype(compStore.end()) iterEnd;
};

template<typename CompStore, typename Second, typename ... CompTypes>
class MultiView {
public:
	MultiView(EntityComponentManager& manager, CompStore& compStore) 
		: manager{ manager }, compStore{ compStore }, iterEnd{ compStore.end() }
	{ }
	template<typename MainIterT, typename Second, typename ... CompTypes>
	class iterator {
	public:
		typedef iterator<MainIterT, Second, CompTypes...> self_type;
		typedef Entity value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(const MainIterT iter, MultiView& vw) 
			: iter{ iter }, view{ vw } 
		{ }
		inline self_type operator++(int junk) {
			do {
				++iter;
			} while (iter != view.iterEnd && (!view.manager.hasComps<Second, CompTypes...>(*iter) || !view.manager.isSpawned(*iter)));
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline value_type operator*() {
			return *iter;
		}
		inline bool operator==(const self_type& rhs) {
			return iter == rhs.iter;
		}
		inline bool operator!=(const self_type& rhs) {
			return iter != rhs.iter;
		}
	private:
		MainIterT iter;
		MultiView& view;
	};
	inline auto begin() {
		auto iter = compStore.begin();
		while (iter != iterEnd && (!manager.hasComps<Second, CompTypes...>(*iter) || !manager.isSpawned(*iter))) {
			++iter;
		}
		return iterator<decltype(compStore.begin()), Second, CompTypes...>(iter, *this);
	}
	inline auto end() {
		return iterator<decltype(compStore.begin()), Second, CompTypes...>(iterEnd, *this);
	}
private:
	EntityComponentManager& manager;
	CompStore& compStore;
	const decltype(compStore.end()) iterEnd;
};

template<typename First, typename Second, typename ... CompTypes>
[[nodiscard]]
inline auto EntityComponentManager::entity_view() {
	static constexpr auto tuple_index = index_in_storagetuple<First, decltype(componentStorageTuple)>::value;
	return MultiView<decltype(std::get<tuple_index>(componentStorageTuple)), Second, CompTypes...>(*this, std::get<tuple_index>(componentStorageTuple));
}

template<typename CompType>
[[nodiscard]]
inline auto EntityComponentManager::entity_view() {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return SingleView(*this, std::get<tuple_index>(componentStorageTuple));
}

// -------- ComponentView implementation -----------------------------------

using Move = Movement;
using Coll = Collider;
using PBody = PhysicsBody;
using TexRef = TextureRef;
class ComponentView {
public:
	ComponentView(EntityComponentManager& manager, Entity index) : manager{ manager }, index{ index } {}
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
	Entity index;
};

__forceinline ComponentView EntityComponentManager::viewComps(Entity index)
{
	return ComponentView(*this, index);
}

inline ComponentView EntityComponentManager::viewComps(EntityId id)
{
	return ComponentView(*this, idToIndex[id.id]);
}

// ------------------------------------------------------------------------

inline bool EntityComponentManager::exists(Entity index) {
	return (index < entityStorageInfo.size() ? entityStorageInfo[index].isValid() : false);
}

inline bool EntityComponentManager::exists(EntityId id)
{
	return exists(idToIndex[id.id]);
}

inline bool EntityComponentManager::isSpawned(Entity ent)
{
	return entityStorageInfo[ent].isSpawned();
}

inline bool EntityComponentManager::isSpawned(EntityId ent)
{
	return entityStorageInfo[idToIndex[ent.id]].isSpawned();
}

inline void EntityComponentManager::despawn(Entity index)
{
	assert(entityStorageInfo[index].isValid() && entityStorageInfo[index].isSpawned());
	if (hasComp<Collider>(index) && hasntComp<Movement>(index)) staticEntitiesChanged = true;	// set static changed flag
	entityStorageInfo[index].setSpawned(false);
}

inline void EntityComponentManager::despawn(EntityId id)
{
	despawn(idToIndex[id.id]);
}

inline void EntityComponentManager::spawn(Entity index)
{
	assert(entityStorageInfo[index].isValid() && !entityStorageInfo[index].isSpawned());
	if (hasComp<Collider>(index) && hasntComp<Movement>(index)) staticEntitiesChanged = true;	// set static changed flag
	entityStorageInfo[index].setSpawned(true);
}

inline void EntityComponentManager::spawn(EntityId id)
{
	spawn(idToIndex[id.id]);
}

inline bool EntityComponentManager::hasID(Entity index)
{
	return index < indexToId.size() && indexToId[index] > 0;
}

inline EntityId EntityComponentManager::getID(Entity index) {
	assert(hasID(index));
	return EntityId(indexToId[index], idVersion[indexToId[index]]);
}

inline bool EntityComponentManager::isIdValid(EntityId entityId)
{
	return entityId.id < idToIndex.size() && idToIndex[entityId.id] > 0 && idVersion[entityId.id] == entityId.version;
}

inline Entity EntityComponentManager::getIndex(EntityId entityId)
{
	assert(isIdValid(entityId));
	return idToIndex[entityId.id];
}
#pragma once
#include <algorithm>
#include <fstream>

#include <vector>
#include <queue>
#include <deque>
#include <bitset>
#include <tuple>
#include <type_traits>
#include <boost/serialization/split_member.hpp>

#include "robin_hood.h"

#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "algo.hpp"

#include "EntityManager.hpp"
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

class EntityComponentManager : public EntityManager {
	template<typename First, typename Second, typename ... CompTypes>
	friend class MultiView;
	template<typename Comp>
	friend class SingleView;
	friend class boost::serialization::access;
public:
	/* creates blank entity and returns it */
	Entity create();
	/* creates blank entity and returns it's id */
	EntityId idCreate();

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
protected:
	template<class Archive>
	void save(Archive& ar, const unsigned int version) const
	{
		ar << boost::serialization::base_object<EntityManager>(*this);
		for_each(componentStorageTuple,
			[&](auto& componentStorage) {
				ar << componentStorage.dump();
			}
		);
	}

	template<class Archive>
	void load(Archive& ar, const unsigned int version)
	{
		ar >> boost::serialization::base_object<EntityManager>(*this);
		for_each(componentStorageTuple,
			[&](auto& componentStorage) {
				componentStorage.updateMaxEntNum(entityStorageInfo.size());
				decltype(componentStorage.dump()) dump;
				ar >> dump;
				for (auto [ent, comp] : dump) {
					componentStorage.insert(ent, comp);
				}
			}
		);
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		boost::serialization::split_member(ar, *this, file_version);
	}
private:
	void updateMaxEntityToComponentSotrages(Entity entity);
	void moveEntity(Entity start, Entity goal);
	void deregisterDestroyedEntities();

	std::tuple<CORE_COMPONENT_SEGMENT, GAME_COMPONENT_SEGMENT> componentStorageTuple;
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
	return getComp<CompType>(idToIndexTable[id.identifier]);
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
	return hasComp<CompType>(idToIndexTable[id.identifier]);
}

template<typename CompType> __forceinline bool EntityComponentManager::hasntComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	return !std::get<tuple_index>(componentStorageTuple).contains(index);
}
template<typename CompType>
inline bool EntityComponentManager::hasntComp(EntityId id)
{
	return hasntComp<CompType>(idToIndexTable[id.identifier]);
}

template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(Entity index, CompType data) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).insert(index, data);
	return std::get<tuple_index>(componentStorageTuple)[index];
}
template<typename CompType>
inline CompType& EntityComponentManager::addComp(EntityId id, CompType data)
{
	return addComp<CompType>(idToIndexTable[id.identifier], data);
}

template<typename CompType> __forceinline CompType& EntityComponentManager::addComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).insert(index, CompType());
	return std::get<tuple_index>(componentStorageTuple)[index];
}
template<typename CompType>
inline CompType& EntityComponentManager::addComp(EntityId id)
{
	return addComp<CompType>(idToIndexTable[id.identifier]);
}


template<typename CompType> __forceinline void EntityComponentManager::remComp(Entity index) {
	static constexpr auto tuple_index = index_in_storagetuple<CompType, decltype(componentStorageTuple)>::value;
	std::get<tuple_index>(componentStorageTuple).remove(index);
}

template<typename CompType>
inline void EntityComponentManager::remComp(EntityId id)
{
	remComp<CompType>(idToIndexTable[id.identifier]);
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
	return hasComps<CompTypes...>(idToIndexTable[id.identifier]);
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
	return hasntComps<CompTypes...>(idToIndexTable[id.identifier]);
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
	return ComponentView(*this, idToIndexTable[id.identifier]);
}

// ------------------------------------------------------------------------

inline void EntityComponentManager::updateMaxEntityToComponentSotrages(Entity entity)
{
	for_each(componentStorageTuple, [&](auto& componentStorage) {
		componentStorage.updateMaxEntNum(entityStorageInfo.size());
		});
}

inline Entity EntityComponentManager::create()
{
	Entity ent = EntityManager::create();
	updateMaxEntityToComponentSotrages(ent);
	return ent;
}

inline EntityId EntityComponentManager::idCreate()
{
	auto index = create();
	return getId(index);
}

inline void EntityComponentManager::deregisterDestroyedEntities()
{
	for (Entity entity : destroyQueue) {
		for_each(componentStorageTuple, [&](auto& componentStorage) {
			componentStorage.remove(entity);
			});
	}
}

inline void EntityComponentManager::moveEntity(Entity start, Entity goal)
{
	assert(entityStorageInfo.size() > goal && entityStorageInfo[goal].isValid() == false);
	assert(entityStorageInfo.size() > start && entityStorageInfo[start].isValid() == true);
	if (hasId(start)) {
		idToIndexTable[indexToIdTable[start]] = goal;
		indexToIdTable[goal] = indexToIdTable[start];
		indexToIdTable[start] = INVALID_ID;
	}
	for_each(componentStorageTuple, [&](auto& componentStorage) {
		if (componentStorage.contains(start)) {
			componentStorage.insert(goal, componentStorage.get(start));
			componentStorage.remove(start);
			assert(componentStorage.contains(goal));
			assert(!componentStorage.contains(start));
		}
		});
	entityStorageInfo[goal].setValid(true);
	entityStorageInfo[goal].setSpawned(entityStorageInfo[start].isSpawned());
	entityStorageInfo[start].setValid(false);
	entityStorageInfo[start].setSpawned(false);
}

inline void EntityComponentManager::defragment(DefragMode const mode)
{
	int maxDefragEntCount;
	float minFragmentation(1.0f);
	switch (mode) {
	case DefragMode::FAST:
		maxDefragEntCount = 10;
		minFragmentation = 0.001;
		break;
	case DefragMode::NONE:
		maxDefragEntCount = 0;
		minFragmentation = 1.01f;
		break;
	case DefragMode::LAZY:
		maxDefragEntCount = 200;
		minFragmentation = 0.5f;
		break;
	case DefragMode::MODERATE:
		maxDefragEntCount = 200;
		minFragmentation = 0.33333f;
		break;
	case DefragMode::EAGER:
		maxDefragEntCount = 100;
		minFragmentation = 0.1f;
		break;
	case DefragMode::AGRESSIVE:
		maxDefragEntCount = 50;
		minFragmentation = 0.01f;
		break;
	case DefragMode::COMPLETE:
		maxDefragEntCount = maxEntityIndex();
		minFragmentation = 0.00001f;
		break;
	}

	if (fragmentation() > minFragmentation) {
		shrink();

		for (int defragCount = 0; defragCount < maxDefragEntCount; defragCount++) {
			if (freeIndexQueue.empty()) break;
			auto biggesthandle = findBiggestValidEntityIndex();
			moveEntity(biggesthandle, freeIndexQueue.front());
			freeIndexQueue.pop_front();
			shrink();
		}
	}
}
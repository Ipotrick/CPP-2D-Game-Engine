#pragma once
#include <algorithm>
#include <fstream>
#include <vector>
#include <queue>
#include <deque>
#include <bitset>
#include <tuple>
#include <type_traits>
#include <functional>
#include <boost/serialization/split_member.hpp>

#include "robin_hood.h"
#include "algo.hpp"
#include "EntityManager.hpp"
#include "ComponentTypes.hpp"

class ComponentView; 
template<typename FirstComp, typename ... OtherComps>
class EntityComponentView;
template<typename FirstComp, typename ... OtherComps>
class EntityView;

class EntityComponentManager : public EntityManager {
	template<typename FirstComp, typename ... OtherComps>
	friend class EntityComponentView;
	template<typename FirstComp, typename ... OtherComps>
	friend class EntityView;
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
	template<typename ... CompType> auto getComps(Entity index);
	template<typename ... CompType> auto getComps(EntityId id);
	/* returns bool whether or not the given index has the component added/registered, ~O(1) */
	template<typename CompType> bool hasComp(Entity index);
	template<typename CompType> bool hasComp(EntityId id);
	template<typename ... CompTypes> bool hasComps(Entity index);
	template<typename ... CompTypes> bool hasComps(EntityId id);
	/* returns bool whether or not the given index has'nt the component added/registered, ~O(1) */
	template<typename CompType>	bool hasntComp(Entity index);
	template<typename CompType> bool hasntComp(EntityId id);
	template<typename ... CompTypes> bool hasntComps(Entity index);
	template<typename ... CompTypes> bool hasntComps(EntityId id);
	/* adds a new Compoenent to an index, ~O(1) */
	template<typename CompType> CompType& addComp(Entity index, CompType data = CompType());
	template<typename CompType> CompType& addComp(EntityId id,	CompType data = CompType());
	/* removes a component from the index */
	template<typename CompType> void remComp(Entity index);
	template<typename CompType> void remComp(EntityId id);
	/* returnes a View, and iterable object that only iterates over the entities with the given Components, ignored despawned entities */
	template<typename FirstComp, typename ... RestComps> [[nodiscard]] auto entityView();
	template<typename FirstComp, typename ... RestComps> [[nodiscard]] auto entityComponentView();
	[[nodiscard]] ComponentView componentView(Entity index);
	[[nodiscard]] ComponentView componentView(EntityId id);

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
	template<typename T> static inline constexpr size_t storageIndex()
	{
		return index_in_storagetuple<T, decltype(componentStorageTuple)>::value;
	}
	template<class Archive> void save(Archive& ar, const unsigned int version) const
	{
		ar << boost::serialization::base_object<EntityManager>(*this);
		tuple_for_each(componentStorageTuple,
			[&](auto& componentStorage) {
				ar << componentStorage.dump();
			}
		);
	}
	template<class Archive> void load(Archive& ar, const unsigned int version)
	{
		ar >> boost::serialization::base_object<EntityManager>(*this);
		tuple_for_each(componentStorageTuple,
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
	template<class Archive> void serialize(Archive& ar, const unsigned int file_version)
	{
		boost::serialization::split_member(ar, *this, file_version);
	}
private:
	void updateMaxEntityToComponentSotrages(Entity entity);
	void moveEntity(Entity start, Entity goal);
	void deregisterDestroyedEntities();

	ComponentStorageTuple componentStorageTuple;
};

// ---- Component Accessors implementations --------------------------------

template<typename CompType> inline auto& EntityComponentManager::getAll() {
	constexpr auto tuple_index = storageIndex<CompType>();
	return std::get<tuple_index>(componentStorageTuple);
} 

template<typename CompType> inline CompType& EntityComponentManager::getComp(Entity index) {
	constexpr auto tuple_index = storageIndex<CompType>();
	return std::get<tuple_index>(componentStorageTuple).get(index);
}
template<typename CompType>
inline CompType& EntityComponentManager::getComp(EntityId id)
{
	return getComp<CompType>(idToIndexTable[id.identifier]);
}

template<typename ... CompType>
inline auto EntityComponentManager::getComps(Entity index)
{
	return std::tuple<CompType&...>(getComp<CompType>(index) ...);
}

template<typename ... CompType>
inline auto EntityComponentManager::getComps(EntityId id)
{
	const Entity ent = idToIndexTable[id.identifier];
	return std::tuple<CompType&...>(getComp<CompType>(ent) ...);
}

template<typename CompType> inline bool EntityComponentManager::hasComp(Entity index) {
	constexpr auto tuple_index = storageIndex<CompType>();
	return std::get<tuple_index>(componentStorageTuple).contains(index);
}
template<typename CompType>
inline bool EntityComponentManager::hasComp(EntityId id)
{
	return hasComp<CompType>(idToIndexTable[id.identifier]);
}

template<typename CompType> inline bool EntityComponentManager::hasntComp(Entity index) {
	constexpr auto tuple_index = storageIndex<CompType>();
	return !std::get<tuple_index>(componentStorageTuple).contains(index);
}
template<typename CompType>
inline bool EntityComponentManager::hasntComp(EntityId id)
{
	return hasntComp<CompType>(idToIndexTable[id.identifier]);
}

template<typename CompType> inline CompType& EntityComponentManager::addComp(Entity index, CompType data) {
	constexpr auto tuple_index = storageIndex<CompType>();
	std::get<tuple_index>(componentStorageTuple).insert(index, data);
	return std::get<tuple_index>(componentStorageTuple)[index];
}
template<typename CompType>
inline CompType& EntityComponentManager::addComp(EntityId id, CompType data)
{
	return addComp<CompType>(idToIndexTable[id.identifier], data);
}


template<typename CompType> inline void EntityComponentManager::remComp(Entity index) {
	constexpr auto tuple_index = storageIndex<CompType>();
	std::get<tuple_index>(componentStorageTuple).remove(index);
}

template<typename CompType>
inline void EntityComponentManager::remComp(EntityId id)
{
	remComp<CompType>(idToIndexTable[id.identifier]);
}

// ---------- hasComps implementation --------------------------------------

template<typename... CompTypes>
inline bool EntityComponentManager::hasComps(Entity index) {
	return (hasComp<CompTypes>(index) && ...);
}

template<typename ...CompTypes>
inline bool EntityComponentManager::hasComps(EntityId id)
{
	return hasComps<CompTypes...>(idToIndexTable[id.identifier]);
}

// --------- hasntComps implementation -------------------------------------

template<typename... CompTypes>
inline bool EntityComponentManager::hasntComps(Entity index) {
	return (hasntComp<CompTypes>(index) && ...);
}

template<typename ...CompTypes>
inline bool EntityComponentManager::hasntComps(EntityId id)
{
	return hasntComps<CompTypes...>(idToIndexTable[id.identifier]);
}

// ------------ view implementation ----------------------------------------

template<typename FirstComp, typename ... RestComps>
class EntityComponentView {
public:
	EntityComponentView(EntityComponentManager& manager)
		: manager{ manager }, compStore{ std::get<manager.storageIndex<FirstComp>()>(manager.componentStorageTuple) }, iterEnd{ compStore.end() }
	{ }
	template<typename MainIterT, typename FirstCompType, typename ... RestCompTypes>
	class iterator {
	public:
		typedef iterator<MainIterT, FirstCompType, RestCompTypes...> self_type;
		typedef std::tuple<Entity, FirstCompType&, RestCompTypes&...> value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(const MainIterT iter, EntityComponentView& vw)
			: iter{ iter }, view{ vw }
		{ }
		inline self_type operator++(int junk)
		{
			do {
				++iter;
			} while (iter != view.iterEnd && (!view.manager.hasComps<RestCompTypes...>(*iter) || !view.manager.isSpawned(*iter)));
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
			return std::tuple_cat(
				std::tuple<Entity, FirstCompType&>(*iter, iter.data()),
				view.manager.getComps<RestCompTypes...>(*iter)
			);
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
		EntityComponentView& view;
	};
	inline auto begin()
	{
		auto iter = compStore.begin();
		while (iter != iterEnd && (!manager.hasComps<RestComps...>(*iter) || !manager.isSpawned(*iter))) {
			++iter;
		}
		return iterator<decltype(compStore.begin()), FirstComp, RestComps...>(iter, *this);
	}
	inline auto end()
	{
		return iterator<decltype(compStore.begin()), FirstComp, RestComps...>(iterEnd, *this);
	}
private:
	EntityComponentManager& manager;
	decltype(std::get<manager.storageIndex<FirstComp>()>(manager.componentStorageTuple))& compStore;
	const decltype(compStore.end()) iterEnd;
};

template<typename FirstComp, typename ... RestComps>
class EntityView {
	EntityComponentManager& manager;
	decltype(std::get<manager.storageIndex<FirstComp>()>(manager.componentStorageTuple))& compStore;
	const decltype(compStore.end()) iterEnd;
public:
	EntityView(EntityComponentManager& manager)
		: manager{ manager }, compStore{ std::get<manager.storageIndex<FirstComp>()>(manager.componentStorageTuple) }, iterEnd{ compStore.end() }
	{ }
	template<typename MainIterT, typename FirstCompType, typename ... RestCompTypes>
	class iterator {
	public:
		typedef iterator<MainIterT, FirstCompType, RestCompTypes...> self_type;
		typedef Entity value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(const MainIterT iter, EntityView& vw)
			: iter{ iter }, view{ vw }
		{ }
		inline self_type operator++(int junk)
		{
			do {
				++iter;
			} while (iter != view.iterEnd && (!view.manager.hasComps<RestCompTypes...>(*iter) || !view.manager.isSpawned(*iter)));
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
		EntityView& view;
	};
	inline auto begin()
	{
		auto iter = compStore.begin();
		while (iter != iterEnd && (!manager.hasComps<RestComps...>(*iter) || !manager.isSpawned(*iter))) {
			++iter;
		}
		return iterator<decltype(compStore.begin()), FirstComp, RestComps...>(iter, *this);
	}
	inline auto end()
	{
		return iterator<decltype(compStore.begin()), FirstComp, RestComps...>(iterEnd, *this);
	}
private:
};

template<typename FirstComp, typename ... RestComps>
[[nodiscard]]
inline auto EntityComponentManager::entityView() 
{
	return EntityView<FirstComp, RestComps...>(*this);
}

template<typename FirstComp, typename ... RestComps>
[[nodiscard]]
inline auto EntityComponentManager::entityComponentView()
{
	return EntityComponentView<FirstComp, RestComps...>(*this);
}

// -------- ComponentView implementation -----------------------------------

using Move = Movement;
using Coll = Collider;
using PBody = PhysicsBody;
using TexRef = TextureRef;
class ComponentView {
public:
	ComponentView(EntityComponentManager& manager, Entity index) 
		: manager{ manager }, index{ index } 
	{}
	template<typename ... CompTypes> [[nodiscard]] bool has()
	{ 
		return manager.hasComps<CompTypes...>(index);
	}
	template<typename CompType> CompType& add(CompType comp = CompType()) 
	{
		return manager.addComp<CompType>(index, comp);
	}
	template<typename ... CompTypes> [[nodiscard]] std::tuple<CompTypes&...> get()
	{ 
		return manager.getComps<CompTypes...>(index);
	}
private:
	EntityComponentManager& manager;
	Entity index;
};

inline ComponentView EntityComponentManager::componentView(Entity index)
{
	return ComponentView(*this, index);
}

inline ComponentView EntityComponentManager::componentView(EntityId id)
{
	return ComponentView(*this, idToIndexTable[id.identifier]);
}

// ------------------------------------------------------------------------

inline void EntityComponentManager::updateMaxEntityToComponentSotrages(Entity entity)
{
	tuple_for_each(componentStorageTuple, [&](auto& componentStorage) {
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
		tuple_for_each(componentStorageTuple, [&](auto& componentStorage) {
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
	tuple_for_each(componentStorageTuple, [&](auto& componentStorage) {
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
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
#include <array>

#include "EntityComponentStorage.hpp"
#include "robin_hood.h"
#include "utils.hpp"
#include "EntityManager.hpp"

template< size_t I, typename T, typename TTuple>
static constexpr size_t findIndexInTuple()
{
	static_assert(I < std::tuple_size<TTuple>::value, "the given component type is unknown");

	using el = typename std::remove_pointer<typename std::remove_reference<typename std::tuple_element<I, TTuple>::type>::type>::type;
	if constexpr (
		std::is_same<ComponentStorageDirectIndexing<T>, el>::value
		|| std::is_same<ComponentStoragePagedIndexing<T>, el>::value
		|| std::is_same<ComponentStoragePagedSet<T>, el>::value
		) {
		return I;
	}
	else {
		return findIndexInTuple<I + 1, T, TTuple>();
	}
}

template< size_t I, typename T, typename TTuple>
static constexpr size_t isTypeInStoreTuple()
{
	if constexpr (I >= std::tuple_size<TTuple>::value) {
		return false;
	}

	using el = typename std::remove_pointer<typename std::remove_reference<typename std::tuple_element<I, TTuple>::type>::type>::type;
	if constexpr (
		std::is_same<ComponentStorageDirectIndexing<T>, el>::value
		|| std::is_same<ComponentStoragePagedIndexing<T>, el>::value
		|| std::is_same<ComponentStoragePagedSet<T>, el>::value
		) {
		return true;
	}
	else {
		return isTypeInStoreTuple<I + 1, T, TTuple>();
	}
}

/**
 * \return if types in tuple are contained in a Storage tuple
 */
template<size_t I, typename TypeTuple, typename StorageTuple>
static constexpr void assertTypeTupleInStorageTuple()
{
	if constexpr (I < std::tuple_size<TypeTuple>::value) {
		using curType = typename std::tuple_element<I, TypeTuple>::type;
		static_assert(isTypeInStoreTuple<0, curType, StorageTuple>(), "tried to use a component type that is not registered in the SubECM!");
		assertTypeTupleInStorageTuple<I + 1, TypeTuple, StorageTuple>();
	}
}

template<typename SubManager, typename FirstComp, typename ... RestComps>
class EntityComponentView2; 
template<typename Manager, typename FirstComp, typename ... RestComps>
class EntityView2; 

template<typename ... CompStoreType>
class SubEntityComponentManager {
	template<typename SubManager, typename FirstComp, typename ... RestComps>
	friend class EntityComponentView2;
	template<typename Manager, typename FirstComp, typename ... RestComps>
	friend class EntityView2;
public:
	SubEntityComponentManager(EntityManager& em, CompStoreType&... comp):
		entManager{&em}, compStorePtrTuple{&comp ...}
	{ }

	SubEntityComponentManager(SubEntityComponentManager<CompStoreType...> const& rhs) :
		entManager{ rhs.entManager }, compStorePtrTuple{ rhs.compStorePtrTuple }
	{ }

	/* EntityManager api */
	EntityHandle create(UUID uuid = UUID())
	{
		return entManager->create(uuid);
	}
	void destroy(EntityHandle entity)
	{
		entManager->destriy(entity);
	}
	void spawnLater(EntityHandle entity)
	{
		entManager->spawnLater(entity);
	}
	void spawn(EntityHandle entity)
	{
		entManager->spawn(entity);
	}
	bool isSpawned(EntityHandleIndex index) const
	{
		return entManager->isSpawned(index);
	}
	bool isSpawned(EntityHandle entity) const
	{
		return entManager->isSpawned(entity);
	}
	void despawn(EntityHandle entity)
	{
		entManager->despawn(entity);
	}

	bool isHandleValid(EntityHandle entity) const
	{
		return entManager->isHandleValid(entity);
	}

	EntityHandle getHandle(EntityHandleIndex index) const
	{
		return entManager->getHandle(index);
	}
	UUID identify(EntityHandle entity)
	{
		return entManager->identify(entity);
	}
	bool exists(UUID id) { 
		return entManager->exists(id);
	}
	EntityHandle getEntity(UUID id)
	{
		return entManager->getEntity(id);
	}
	bool hasId(EntityHandle entity)
	{
		return entManager->hasId(entity);
	}

	size_t const size()
	{
		return entManager->size();
	}
	size_t const maxEntityIndex()
	{
		return entManager->maxEntityIndex();
	}

	/* per entity component access */
	template<typename CompType>		CompType& getComp(EntityHandleIndex index)
	{
		return storage<CompType>().get(index);
	}
	template<typename CompType>		CompType& getComp(EntityHandle entity)
	{
		return getComp<CompType>(entity.index);
	}

	template<typename ... CompType> std::tuple<CompType&...> getComps(EntityHandleIndex index)
	{
		return std::tuple<CompType&...>(getComp<CompType>(index) ...);
	}
	template<typename ... CompType> std::tuple<CompType&...> getComps(EntityHandle entity)
	{
		return getComps<CompType...>(entity.index);
	}

	template<typename CompType>		bool hasComp(EntityHandleIndex index)
	{
		return storage<CompType>().contains(index);
	}
	template<typename CompType>		bool hasComp(EntityHandle entity)
	{
		return hasComp<CompType>(entity.index);
	}
	
	template<> bool hasComp<void>(EntityHandle entity)
	{
		return true;
	}

	template<typename... CompTypes> bool hasComps(EntityHandleIndex index)
	{
		return (hasComp<CompTypes>(index) && ...);
	}
	template<typename... CompTypes> bool hasComps(EntityHandle entity)
	{
		return hasComps<CompTypes...>(entity.index);
	}

	template<typename CompType>		bool hasntComp(EntityHandleIndex index)
	{
		return !storage<CompType>().contains(index);
	}
	template<typename CompType>		bool hasntComp(EntityHandle entity)
	{
		return hasntComp<CompType>(entity);
	}

	template<> bool hasntComp<void>(EntityHandle entity)
	{
		retirm false;
	}

	template<typename... CompTypes> bool hasntComps(EntityHandleIndex index)
	{
		return (hasntComp<CompTypes>(index) && ...);
	}
	template<typename... CompTypes> bool hasntComps(EntityHandle entity)
	{
		return hasntComps<CompTypes...>(entity.index);
	}

	template<typename CompType>		CompType& addComp(EntityHandleIndex index, CompType data = CompType())
	{
		storage<CompType>().insert(index, data);
		return storage<CompType>().get(index);
	}
	template<typename CompType>		CompType& addComp(EntityHandle entity, CompType data = CompType())
	{
		return addComp<CompType>(entity.index, data);
	}

	template<typename CompType>		void remComp(EntityHandleIndex index)
	{
		storage<CompType>().remove(index);
	}
	template<typename CompType>		void remComp(EntityHandle entity)
	{
		remComp<CompType>(entity.index);
	}

	template<typename FirstComp, typename ... RestComps>
	[[nodiscard]]
	auto entityView()
	{
		return EntityView2<SubEntityComponentManager<CompStoreType...>, FirstComp, RestComps...>(*this);
	}

	template<typename FirstComp, typename ... RestComps>
	[[nodiscard]]
	auto entityComponentView()
	{
		assertTypeTupleInStorageTuple<0, std::tuple<FirstComp, RestComps...>, std::tuple<CompStoreType...>>();
		return EntityComponentView2<SubEntityComponentManager<CompStoreType...>, FirstComp, RestComps...>(*this);
	}
private:

	bool isIndexValid(EntityHandleIndex index) const
	{
		return isIndexValid(index);
	}

	EntityHandleVersion getVersion(EntityHandleIndex index)
	{
		return entManager->getVersion(index);
	}

	template<typename CompType>
	constexpr auto& storage()
	{
		constexpr int index = findIndexInTuple<0, CompType, std::tuple<CompStoreType...>>();
		return *std::  get<index>(compStorePtrTuple);
	}

	EntityManager* entManager;
	std::tuple<CompStoreType*...> compStorePtrTuple;
};

template<typename SubManager, typename FirstComp, typename ... RestComps>
class EntityComponentView2 {
public:
	EntityComponentView2(SubManager manager)
		: manager{ manager }, compStore{ manager.storage<FirstComp>() }, iterEnd{ compStore.end() }
	{ }
	template<typename MainIterT, typename FirstCompType, typename ... RestCompTypes>
	class iterator {
	public:
		typedef iterator<MainIterT, FirstCompType, RestCompTypes...> self_type;
		typedef std::tuple<EntityHandle, FirstCompType&, RestCompTypes&...> value_type;
		typedef EntityHandle& reference;
		typedef EntityHandle* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(const MainIterT iter, EntityComponentView2<SubManager, FirstComp, RestComps...>& vw)
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
				std::tuple<EntityHandle, FirstCompType&>(EntityHandle{ *iter, view.manager.getVersion(*iter) }, iter.data()),
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
		EntityComponentView2<SubManager, FirstComp, RestComps...>& view;
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
	SubManager manager;
	decltype(manager.storage<FirstComp>())& compStore;
	const decltype(compStore.end()) iterEnd;
};


template<typename Manager, typename FirstComp, typename ... RestComps>
class EntityView2 {
public:
	EntityView2(Manager manager)
		: manager{ manager }, compStore{ manager.storage<FirstComp>() }, iterEnd{ compStore.end() }
	{ }
	template<typename MainIterT, typename FirstCompType, typename ... RestCompTypes>
	class iterator {
	public:
		typedef iterator<MainIterT, FirstCompType, RestCompTypes...> self_type;
		typedef EntityHandle value_type;
		typedef EntityHandle& reference;
		typedef EntityHandle* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(const MainIterT iter, EntityView2& vw)
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
			return EntityHandle{ *iter, view.manager.getVersion(*iter) };
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
		EntityView2& view;
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
	Manager manager;
	decltype(manager.storage<FirstComp>())& compStore;
	const decltype(compStore.end()) iterEnd;
};
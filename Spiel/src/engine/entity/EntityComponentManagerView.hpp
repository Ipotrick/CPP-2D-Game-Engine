#pragma once

#include <algorithm>
#include <vector>
#include <queue>
#include <deque>
#include <tuple>
#include <functional>
#include <array>

#include "EntityComponentStorage.hpp"
#include "EntityManager.hpp"

template<size_t I, typename T, typename TTuple>
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
class EntityComponentView; 
template<typename ECMView, typename FirstComp, typename ... RestComps>
class EntityView; 

template<typename ... CompStoreType>
class EntityComponentManagerView {
	template<typename SubManager, typename FirstComp, typename ... RestComps>
	friend class EntityComponentView;
	template<typename ECMView, typename FirstComp, typename ... RestComps>
	friend class EntityView;
public:
	EntityComponentManagerView(EntityManager& em, CompStoreType&... comp):
		entManager{&em}, compStorePtrTuple{&comp ...}
	{ }

	EntityComponentManagerView(EntityComponentManagerView<CompStoreType...> const& rhs) :
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
		return EntityView<EntityComponentManagerView<CompStoreType...>, FirstComp, RestComps...>(*this);
	}

	template<typename FirstComp, typename ... RestComps>
	[[nodiscard]]
	auto entityComponentView()
	{
		assertTypeTupleInStorageTuple<0, std::tuple<FirstComp, RestComps...>, std::tuple<CompStoreType...>>();
		return EntityComponentView<EntityComponentManagerView<CompStoreType...>, FirstComp, RestComps...>(*this);
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
		return *std::get<findIndexInTuple<0, CompType, std::tuple<CompStoreType...>>()>(compStorePtrTuple);
	}

	EntityManager* entManager;
	std::tuple<CompStoreType*...> compStorePtrTuple;
};

/*----------------------------------------------------------------------------------*/
/*-------------------------------EntityComponentView--------------------------------*/
/*----------------------------------------------------------------------------------*/

template<typename ECMView, typename FirstComp, typename ... RestComp>
class EntityComponentView {
public:
	EntityComponentView(ECMView manager)
		: manager{ manager }, compStore{ manager.storage<FirstComp>() }, iterEnd{ compStore.end() }
	{ }

	template<typename MainIterT>
	class iterator {
	public:
		using self_type				= iterator<MainIterT>;
		using value_type			= std::tuple<EntityHandle, FirstComp&, RestComp&...>;
		using reference				= EntityHandle&;
		using pointer				= EntityHandle*;
		using iterator_category		= std::forward_iterator_tag;

		iterator(const MainIterT iter, EntityComponentView<ECMView, FirstComp, RestComp...>& vw)
			: iter{ iter }, view{ vw }
		{ }
		self_type operator++()
		{
			do {
				++iter;
			} while (iter != view.iterEnd && (!view.manager.hasComps<RestComp...>(*iter) || !view.manager.isSpawned(*iter)));
			return *this;
		}
		self_type operator++(int junk)
		{
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		value_type operator*()
		{
			return std::tuple_cat(
				std::tuple<EntityHandle, FirstComp&>(EntityHandle{ *iter, view.manager.getVersion(*iter) }, iter.data()),
				view.manager.getComps<RestComp...>(*iter)
			);
		}
		bool operator==(const self_type& rhs) const
		{
			return iter == rhs.iter;
		}
		bool operator!=(const self_type& rhs) const
		{
			return iter != rhs.iter;
		}
	private:
		MainIterT iter;
		EntityComponentView<ECMView, FirstComp, RestComp...>& view;
	};

	auto begin()
	{
		auto iter = compStore.begin();
		while (iter != iterEnd && (!manager.hasComps<RestComp...>(*iter) || !manager.isSpawned(*iter))) {
			++iter;
		}
		return iterator(iter, *this);
	}

	auto end()
	{
		return iterator(iterEnd, *this);
	}
protected:
	ECMView manager;
	decltype(manager.storage<FirstComp>())& compStore;
	const decltype(compStore.end()) iterEnd;
};

/*----------------------------------------------------------------------------------*/
/*------------------------------------EntityView------------------------------------*/
/*----------------------------------------------------------------------------------*/

template<typename ECMView, typename FirstComp, typename ... RestComp>
class EntityView {
public:
	EntityView(ECMView manager)
		: manager{ manager }, compStore{ manager.storage<FirstComp>() }, iterEnd{ compStore.end() }
	{ }
	template<typename MainIterT>
	class iterator {
	public:
		using self_type = iterator<MainIterT>;
		using value_type = EntityHandle;
		using reference = EntityHandle&;
		using pointer = EntityHandle*;
		using iterator_category = std::forward_iterator_tag;

		iterator(const MainIterT iter, EntityView& vw)
			: iter{ iter }, view{ vw }
		{ }
		self_type operator++()
		{
			do {
				++iter;
			} while (iter != view.iterEnd && (!view.manager.hasComps<RestComp...>(*iter) || !view.manager.isSpawned(*iter)));
			return *this;
		}
		self_type operator++(int junk)
		{
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		value_type operator*()
		{
			return EntityHandle{ *iter, view.manager.getVersion(*iter) };
		}
		bool operator==(const self_type& rhs) const
		{
			return iter == rhs.iter;
		}
		bool operator!=(const self_type& rhs) const
		{
			return iter != rhs.iter;
		}
	private:
		MainIterT iter;
		EntityView& view;
	};
	auto begin()
	{
		auto iter = compStore.begin();
		while (iter != iterEnd && (!manager.hasComps<RestComp...>(*iter) || !manager.isSpawned(*iter))) {
			++iter;
		}
		return iterator(iter, *this);
	}
	auto end()
	{
		return iterator(iterEnd, *this);
	}
private:
	ECMView manager;
	decltype(manager.storage<FirstComp>())& compStore;
	const decltype(compStore.end()) iterEnd;
};
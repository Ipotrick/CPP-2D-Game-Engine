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

#include "robin_hood.h"
#include "utils.hpp"
#include "EntityManager.hpp"
#include "ComponentTypes.hpp"

class EntityComponentManager : public EntityManager {
public:

	/* creates blank entity and returns it */
	EntityHandle create(UUID uuid = UUID());

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

	template<typename... CompTypes> bool hasntComps(EntityHandleIndex index)
	{
		return (hasntComp<CompTypes>(index) && ...);
	}
	template<typename... CompTypes> bool hasntComps(EntityHandle entity)
	{
		return hasntComps<CompTypes...>(entity.index);
	}

	template<typename CompType>		CompType&	addComp(EntityHandleIndex index, CompType data = CompType())
	{
		storage<CompType>().insert(index, data);
		return storage<CompType>().get(index);
	}
	template<typename CompType>		CompType&	addComp(EntityHandle entity, CompType data = CompType())
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

	[[nodiscard]]
	ComponentView	componentView(EntityHandle entity);

	/* general entity and storage access */
	template<typename FirstComp, typename ... RestComps> 
	[[nodiscard]] 
	auto entityView()
	{
		return EntityView<FirstComp, RestComps...>(*this);
	}

	template<typename FirstComp, typename ... RestComps> 
	[[nodiscard]] 
	auto entityComponentView()
	{
		return EntityComponentView<FirstComp, RestComps...>(*this);
	}

	void update();

protected:
	template<typename FirstComp, typename ... OtherComps> 
	friend class EntityComponentView;
	template<typename FirstComp, typename ... OtherComps> 
	friend class EntityView;
	friend class YAMLWorldSerializer;
	friend class Serializer;

	template<typename CompType> 
	constexpr auto& storage()
	{
		return std::get<findIndexInComponentStorageTuple<0, CompType>()>(componentStorageTuple);
	}

	void updateMaxEntityToComponentSotrages();

	void deregisterDestroyedEntities();

	friend ComponentStorageTuple& getNakedStorage(EntityComponentManager& manager);
	ComponentStorageTuple componentStorageTuple;
};

template<> inline bool EntityComponentManager::hasComp<void>(EntityHandle entity)
{
	return true;
}

template<> inline bool EntityComponentManager::hasntComp<void>(EntityHandle entity)
{
	return true;
}

template<typename FirstComp, typename ... RestComps>
class EntityComponentView {
public:
	EntityComponentView(EntityComponentManager& manager)
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
	decltype(manager.storage<FirstComp>())& compStore;
	const decltype(compStore.end()) iterEnd;
};

template<typename FirstComp, typename ... RestComps>
class EntityView {
public:
	EntityView(EntityComponentManager& manager)
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
	EntityComponentManager& manager;
	decltype(manager.storage<FirstComp>())& compStore;
	const decltype(compStore.end()) iterEnd;
};

// -------- ComponentView implementation -----------------------------------

using Move = Movement;
using Coll = Collider;
using PBody = PhysicsBody;
using TexRef = SmallTextureRef;
class ComponentView {
public:
	ComponentView(EntityComponentManager& manager, EntityHandle entity) 
		: manager{ manager }, entity{ entity }
	{}
	template<typename ... CompTypes> 
	[[nodiscard]] 
	bool has() {  return manager.hasComps<CompTypes...>(entity); }
	template<typename CompType> 
	CompType& add(CompType comp = CompType()) { return manager.addComp<CompType>(entity, comp); }
	template<typename CompType> 
	[[nodiscard]] 
	CompType& get() {  return manager.getComp<CompType>(entity); }
private:
	EntityComponentManager& manager;
	EntityHandle entity;
};

inline ComponentView EntityComponentManager::componentView(EntityHandle entity) { return ComponentView(*this, entity); }

/*
* This breaks encapsulation. Please only use it for serialization
*/
inline ComponentStorageTuple& getNakedStorage(EntityComponentManager& manager)
{
	return manager.componentStorageTuple;
}
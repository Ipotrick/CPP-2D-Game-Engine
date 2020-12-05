#pragma once

#include "EntityComponentManagerView.hpp"

template<class ... TComponentStorage>
class EntityComponentManager : public EntityManager {
	using CompStoreTupleType = std::tuple<TComponentStorage...>;
public:

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

	template<> bool hasComp<void>(EntityHandle entity)
	{
		return true;
	}
	template<> bool hasntComp<void>(EntityHandle entity)
	{
		return true;
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

	class ComponentView {
	public:
		ComponentView(EntityComponentManager<TComponentStorage...>& manager, EntityHandle entity)
			: manager{ manager }, entity{ entity }
		{}
		template<typename ... CompTypes>
		[[nodiscard]]
		bool has() { return manager.hasComps<CompTypes...>(entity); }
		template<typename CompType>
		CompType& add(CompType comp = CompType()) { return manager.addComp<CompType>(entity, comp); }
		template<typename CompType>
		[[nodiscard]]
		CompType& get() { return manager.getComp<CompType>(entity); }
	private:
		EntityComponentManager<TComponentStorage...>& manager;
		EntityHandle entity;
	};

	[[nodiscard]] ComponentView componentView(EntityHandle entity)
	{
		return ComponentView(*this, entity);
	}

	template<typename ... CompType>
	auto submodule()
	{
		return SubEntityComponentManager(*static_cast<EntityManager*>(this), storage<CompType>() ...);
	}

	/* general entity and storage access */
	template<typename FirstComp, typename ... RestComps> 
	[[nodiscard]] 
	auto entityView()
	{
		auto subManager = submodule<FirstComp, RestComps...>();
		return subManager.entityView<FirstComp, RestComps...>();
	}

	template<typename FirstComp, typename ... RestComps> 
	[[nodiscard]] 
	auto entityComponentView()
	{
		auto subManager = submodule<FirstComp, RestComps...>();
		return subManager.entityComponentView<FirstComp, RestComps...>();
	}

	void update()
	{
		executeDelayedSpawns();
		deregisterDestroyedEntities();
		executeDestroys();
	}

protected:

	template<typename CompType> 
	auto& storage()
	{
		return std::get<findIndexInTuple<0, CompType, CompStoreTupleType>()>(componentStorageTuple);
	}

	void deregisterDestroyedEntities()
	{
		util::tuple_for_each(componentStorageTuple,
			[&](auto& componentStorage) {
				for (EntityHandleIndex entity : destroyQueue) {
					if (componentStorage.contains(entity)) {
						componentStorage.remove(entity);
					}
				}
			}
		);
	}

	CompStoreTupleType componentStorageTuple;
};
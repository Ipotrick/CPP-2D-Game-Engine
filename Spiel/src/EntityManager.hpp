#pragma once

#include <array>
#include <vector>
#include <deque>
#include <cstdint>
#include <bitset>
#include <cassert>

#include "EntityTypes.hpp"

class YAMLWorldSerializer;
class ComponentView;
template<typename FirstComp, typename ... RestComps> class EntityComponentView; 
template<typename FirstComp, typename ... RestComps> class EntityView;

class EntityManager {
public:

	EntityHandle create();
	void destroy(EntityHandle entity);
	void spawnLater(EntityHandle entity);
	void spawn(EntityHandle entity)
	{
		assert(isHandleValid(entity));
		entitySlots[entity.index].setSpawned(true);
	}
	bool isSpawned(EntityHandleIndex index) const
	{
		return isIndexValid(index) && entitySlots[index].isSpawned();
	}
	bool isSpawned(EntityHandle entity) const
	{
		return isHandleValid(entity) && entitySlots[entity.index].isSpawned();
	}
	void despawn(EntityHandle entity)
	{
		assert(isHandleValid(entity));
		entitySlots[entity.index].setSpawned(false);
	}

	bool isHandleValid(EntityHandle entity) const
	{
		return isIndexValid(entity.index) && entitySlots[entity.index].version == entity.version;
	}

	EntityHandle getHandle(EntityHandleIndex index) const
	{
		assert(isIndexValid(index));
		return EntityHandle{ index, entitySlots[index].version };
	}

	/* returns count of entities */
	size_t const size();
	/* returns the capacity-1 / the biggest possible entity index */
	size_t const maxEntityIndex();

protected:
	friend class YAMLWorldSerializer;
	friend class ComponentView;
	template<typename FirstComp, typename ... RestComps> friend class EntityComponentView; 
	template<typename FirstComp, typename ... RestComps> friend class EntityView;

	bool isIndexValid(EntityHandleIndex index) const
	{
		return (size_t)index < entitySlots.size()
			&& entitySlots[index].holdsEntity();
	}

	EntityHandleVersion getVersion(EntityHandleIndex index)
	{
		assert(isIndexValid(index));
		return entitySlots[index].version;
	}

	void executeDelayedSpawns();
	void executeDestroys();
	EntityHandleIndex findBiggestValidEntityIndex();
	class EntitySlot {
	public:
		EntitySlot(bool entityExists = false) : flags{}
		{
			flags[0] = entityExists;
			flags[1] = false;
		}
		inline void setHoldsEntity(bool entityExists) { flags[0] = entityExists; }
		inline bool holdsEntity() const { return flags[0]; }
		inline void setSpawned(bool spawned) { flags[1] = spawned; }
		inline bool isSpawned() const { return flags[1]; }

		EntityHandleVersion version{ 0 };
	private:
		// flag 0: does the entityslot hold an entity or is it empty
		// flag 1: spawned
		std::bitset<2> flags;
	};

	std::vector<EntitySlot> entitySlots;
	std::deque<EntityHandleIndex> freeIndexQueue;
	std::vector<EntityHandleIndex> destroyQueue;
	std::vector<EntityHandle> spawnLaterQueue;
};
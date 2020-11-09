#pragma once

#include <array>
#include <vector>
#include <deque>
#include <cstdint>
#include <bitset>
#include <cassert>

#include "robin_hood.h"

#include "UUID.hpp"
#include "EntityTypes.hpp"

#ifdef _DEBUG
#define DEBUG_ENTITY_MANAGER
#endif

#ifdef DEBUG_ENTITY_MANAGER
#define assertEntityManager(x) if(!(x)) throw new std::exception()
#else
#define assertEntityManager(x)
#endif

class YAMLWorldSerializer;
class ComponentView;
template<typename FirstComp, typename ... RestComps> class EntityComponentView; 
template<typename FirstComp, typename ... RestComps> class EntityView;

class EntityManager {
public:

	EntityHandle create(UUID uuid = UUID());
	void destroy(EntityHandle entity);
	void spawnLater(EntityHandle entity);
	void spawn(EntityHandle entity)
	{
		assertEntityManager(isHandleValid(entity));
		entitySlots[entity.index].valid = true;
	}
	bool isSpawned(EntityHandleIndex index) const
	{
		assertEntityManager(isIndexValid(index));
		return entitySlots[index].valid;
	}
	bool isSpawned(EntityHandle entity) const
	{
		assertEntityManager(isHandleValid(entity));
		return entitySlots[entity.index].spawned;
	}
	void despawn(EntityHandle entity)
	{
		assertEntityManager(isHandleValid(entity));
		entitySlots[entity.index].spawned = false;
	}

	bool isHandleValid(EntityHandle entity) const
	{
		return isIndexValid(entity.index) && entitySlots[entity.index].version == entity.version;
	}

	EntityHandle getHandle(EntityHandleIndex index) const
	{
		assertEntityManager(isIndexValid(index));
		return EntityHandle{ index, entitySlots[index].version };
	}

	/*
	* !!WARNING EXPENSIVE OPERATION!!
	* returns a uuid for the entity.
	* if the entity does not have a uuid, a new one is generated and asigned
	*/
	UUID identify(EntityHandle entity)
	{
		assertEntityManager(isHandleValid(entity));
		// when entity has no uuid, one is generated on the fly
		if (!entitySlots[entity.index].uuid.isValid()) {
			entitySlots[entity.index].uuid = generateUUID();
			uuidToEntityIndex[entitySlots[entity.index].uuid] = entity.index;
		}
		return entitySlots[entity.index].uuid;
	}
	/*
	* !!WARNING EXPENSIVE OPERATION!!
	* checks if entity with given uuid is present in EntityHandler
	*/
	bool exists(UUID id) { return uuidToEntityIndex.contains(id); }
	/*
	* !!WARNING EXPENSIVE OPERATION!!
	* returns entityhandle of entity with given uuid
	*/
	EntityHandle getEntity(UUID id)
	{
		assert(exists(id));
		EntityHandleIndex index = uuidToEntityIndex[id];
		return EntityHandle{ index, entitySlots[index].version };
	}
	bool hasId(EntityHandle entity) {
		assertEntityManager(isHandleValid(entity));
		return entitySlots[entity.index].uuid.isValid();
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
			&& entitySlots[index].valid;
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
		EntitySlot(bool entityExists = false)
		{
			valid = entityExists;
			spawned = false;
		}

		UUID uuid;
		EntityHandleVersion version{ 0 };
		bool valid{ false };
		bool spawned{ false };
	private:
	};

	std::vector<EntitySlot> entitySlots;
	std::deque<EntityHandleIndex> freeIndexQueue;
	std::vector<EntityHandleIndex> destroyQueue;
	std::vector<EntityHandle> spawnLaterQueue;
	robin_hood::unordered_map<UUID, EntityHandleIndex> uuidToEntityIndex;
};
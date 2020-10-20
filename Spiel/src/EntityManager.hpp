#pragma once

#include <array>
#include <vector>
#include <deque>
#include <cstdint>
#include <bitset>
#include <cassert>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/access.hpp>

#include "EntityTypes.hpp"

class EntityManager {
public:

	// created new entity and returns it
	Entity create();
	// creates new entity and returns it's id
	EntityId idCreate()
	{
		auto index = create();
		return getId(index);
	}
	bool exists(Entity entity)
	{
		return (entity < entityStatusVec.size() ? entityStatusVec[entity].isValid() : false);
	}
	bool exists(EntityId id)
	{
		return exists(idToIndexTable[id.identifier]);
	}
	/* marks index for deletion, entities are deleted after each update, O(1) */
	void destroy(Entity index);
	void destroy(EntityId id);
	/* delayed(after update call) spawn of an index */
	void spawnLater(Entity index);
	void spawnLater(EntityId id);
	void spawn(Entity index)
	{
		assert(entityStatusVec[index].isValid() && !entityStatusVec[index].isSpawned());
		entityStatusVec[index].setSpawned(true);
	}
	void spawn(EntityId id)
	{
		spawn(idToIndexTable[id.identifier]);
	}
	bool isSpawned(Entity ent)
	{
		return entityStatusVec[ent].isSpawned();
	}
	bool isSpawned(EntityId id)
	{
		return entityStatusVec[idToIndexTable[id.identifier]].isSpawned();
	}
	void despawn(Entity index)
	{
		assert(entityStatusVec[index].isValid() && entityStatusVec[index].isSpawned());
		entityStatusVec[index].setSpawned(false);
	}
	void despawn(EntityId id)
	{
		despawn(idToIndexTable[id.identifier]);
	}

	/* an id has an identifier (uint32_t) and a version. an id is only valid when the identifier and the version are the same in the id and the Managers regestry */
	bool isIdValid(EntityId entityId)
	{
		return idToIndexTable[entityId.identifier] < INVALID_ENTITY && entityId.identifier < idToIndexTable.size() && idToVersionTable[entityId.identifier] == entityId.version;
	}
	EntityId getId(Entity entity)
	{
		return EntityId(indexToIdTable[entity], idToVersionTable[indexToIdTable[entity]]);
	}
	/* this function does not check if the id is valid */
	Entity getIndex(EntityId entityId)
	{
		assert(isIdValid(entityId));
		return idToIndexTable[entityId.identifier];
	}

	/* returns count of entities */
	size_t const size();
	/* returns the capacity-1 / the biggest possible entity index */
	size_t const maxEntityIndex();
	/* returnes how fragmented the entities are */
	float fragmentation();

	bool isIdDynamic(const entity_id_t id) const
	{
		return id & 1;
	}

protected:
	/* !! handles must be sorted */
	void shrink();
	void executeDelayedSpawns();
	void executeDestroys();
	Entity findBiggestValidEntityIndex();
	/* generates new DYNAMIC id for index or returns existing id */
	EntityId makeDynamicId(Entity index);
	/* generates new STATIC id for index or returns existing id */
	EntityId makeStaticId(Entity index);

	bool hasId(Entity index)
	{
		return indexToIdTable[index] < INVALID_ID && index < indexToIdTable.size();
	}
	class EntityStatus {
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int file_version)
		{
			ar& flags;
		}
	public:
		EntityStatus(bool valid = false) : flags{}
		{
			flags[0] = valid;
			flags[1] = false;
		}
		inline void setValid(bool valid) { flags[0] = valid; }
		inline bool isValid() { return flags[0]; }
		inline void setSpawned(bool spawned) { flags[1] = spawned; }
		inline bool isSpawned() { return flags[1]; }

	private:
		// flag 0: valid
		// flag 1: spawned
		std::bitset<2> flags;
	};

	std::vector<EntityStatus> entityStatusVec;
	std::deque<Entity> freeIndexQueue;
	/*
		Notable infos about id's:
		  id & 1  => static id
		!(id & 1) => dynamic id
	*/
	std::vector<Entity>		idToIndexTable;
	std::vector<uint32_t>	idToVersionTable;
	std::vector<entity_id_t>indexToIdTable;
	std::deque<entity_id_t>	freeDynamicIdQueue;
	std::deque<entity_id_t>	freeStaticIdQueue;

	std::vector<Entity> destroyQueue;
	std::vector<Entity> spawnLaterQueue;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& entityStatusVec;
		ar& idToIndexTable;
		ar& idToVersionTable;
		ar& indexToIdTable;
		ar& freeDynamicIdQueue;
		ar& destroyQueue;
		ar& spawnLaterQueue;
	}
private:
};
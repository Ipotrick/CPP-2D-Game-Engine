#pragma once

#include <cstdint>

#include "robin_hood.h"

#include "EntityComponentManager.hpp"
#include "collision_detection.hpp"
#include "Physics.hpp"

#define DEBUG_CONSTRAINT_SET

#ifdef DEBUG_CONSTRAINT_SET
#define constraintset_assert(x, y) if (!(x)) { throw new std::exception(y); }
#else
#define constraintset_assert(x, y)
#endif


inline uint64_t makeConstraintKey(EntityId a, EntityId b)
{
	constraintset_assert(a.identifier < b.identifier, "error: id's must be in order!");
	return (uint64_t)a.identifier << 32 | (uint64_t)b.identifier;
}

inline std::pair<entity_id_t, entity_id_t> decompConstraintKey(uint64_t key)
{
	return { (uint32_t)(key >> 32), (uint32_t)key };
}

class CollisionConstraintSet {
public:
	CollisionConstraint* getIfContains(const EntityId a, const EntityId b);
	bool contains(const EntityId a, const EntityId b);
	CollisionConstraint& get(const EntityId a, const EntityId b);
	void insert(const EntityId a, const EntityId b, const CollisionInfo& collinfo);
	void insert(const EntityId a, const EntityId b, const CollisionConstraint& constraint);
	void erase(const EntityId a, const EntityId b);
	void erase(const uint32_t index);
	auto begin()
	{
		return constraints.begin();
	}
	auto end()
	{
		return constraints.end();
	}
	CollisionConstraint& operator [] (uint64_t index)
	{
		return constraints[index];
	}
	size_t size()
	{
		return constraints.size();
	}
	size_t capacity()
	{
		return constraints.capacity();
	}
private:
	robin_hood::unordered_map<uint64_t, uint32_t> lookupMap;
	std::vector<CollisionConstraint> constraints;
};
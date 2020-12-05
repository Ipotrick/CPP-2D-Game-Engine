#pragma once

#include <cstdint>

#include "robin_hood.h"

#include "../../engine/entity/EntityTypes.hpp"
#include "../collision/collision_detection.hpp"
#include "Physics.hpp"

#define DEBUG_CONSTRAINT_SET

#ifdef DEBUG_CONSTRAINT_SET
#define constraintset_assert(x, y) if (!(x)) { throw new std::exception(y); }
#else
#define constraintset_assert(x, y)
#endif


inline uint64_t makeConstraintKey(EntityHandle a, EntityHandle b)
{
	constraintset_assert(a.index < b.index, "error: id's must be in order!");
	return (uint64_t)a.index << 32 | (uint64_t)b.index;
}

inline std::pair<EntityHandleIndex, EntityHandleIndex> decompConstraintKey(uint64_t key)
{
	return { (uint32_t)(key >> 32), (uint32_t)key };
}

class CollisionConstraintSet {
public:
	CollisionConstraint* getIfContains(const EntityHandle a, const EntityHandle b);
	bool contains(const EntityHandle a, const EntityHandle b);
	CollisionConstraint& get(const EntityHandle a, const EntityHandle b);
	void insert(const EntityHandle a, const EntityHandle b, const CollisionInfo& collinfo);
	void insert(const EntityHandle a, const EntityHandle b, const CollisionConstraint& constraint);
	void erase(const EntityHandle a, const EntityHandle b);
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
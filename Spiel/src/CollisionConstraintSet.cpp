#include "CollisionConstraintSet.hpp"

CollisionConstraint* CollisionConstraintSet::getIfContains(const EntityHandle a, const EntityHandle b)
{
    auto iter = lookupMap.find(makeConstraintKey(a, b));
    if (iter != lookupMap.end() && constraints[iter->second].idA == a && constraints[iter->second].idB == b)
        return  &constraints[iter->second];
    else
        return nullptr;
}

bool CollisionConstraintSet::contains(const EntityHandle a, const EntityHandle b)
{
    uint64_t key = makeConstraintKey(a, b);
    if (lookupMap.contains(key)) {
        auto index = lookupMap[key];
        CollisionConstraint val = constraints[index];
        return a.version == val.idA.version & b.version == val.idB.version;
    }
    else {
        return false;
    }
}

CollisionConstraint& CollisionConstraintSet::get(const EntityHandle a, const EntityHandle b)
{
    constraintset_assert(contains(a, b), "error: cannot retrieve not existing value!");
    return constraints[lookupMap[makeConstraintKey(a, b)]];
}

void CollisionConstraintSet::insert(const EntityHandle a, const EntityHandle b, const CollisionInfo& collinfo)
{
    constraintset_assert(!contains(a, b), "error: cannot insert existing value!");
    auto constraint = CollisionConstraint();
    constraint.idA = a;
    constraint.idB = b;
    constraint.updated = true;
    constraint.collisionPoints[0].normal = collinfo.normal[0];
    constraint.collisionPoints[1].normal = collinfo.normal[1];
    constraint.collisionPoints[0].position = collinfo.position[0];
    constraint.collisionPoints[1].position = collinfo.position[1];
    constraint.collisionPointNum = collinfo.collisionPointNum;
    constraint.clippingDist = collinfo.clippingDist;
    constraints.push_back(constraint);
    const auto key = makeConstraintKey(a, b);
    lookupMap.insert({ key, (uint32_t)(constraints.size() - 1) });
}

void CollisionConstraintSet::insert(EntityHandle a, EntityHandle b, const CollisionConstraint& constraint)
{
    constraintset_assert(!contains(a, b), "error: cannot insert existing value!");
    constraints.push_back(constraint);
    const auto key = makeConstraintKey(a, b);
    lookupMap.insert({ key, (uint32_t)(constraints.size() - 1) });
}

void CollisionConstraintSet::erase(const EntityHandle a, const EntityHandle b)
{
    constraintset_assert(contains(a, b), "error: cannot remove not existing value!");
    const auto key = makeConstraintKey(a, b);
    const auto index = lookupMap[key];
    lookupMap.erase(key);
    if (index < constraints.size() - 1) {
        auto& last = constraints[constraints.size() - 1];
        constraints[index] = last;
        auto keyLast = makeConstraintKey(last.idA, last.idB);
        lookupMap[keyLast] = index;
    }
    constraints.pop_back();
}

void CollisionConstraintSet::erase(const uint32_t index)
{
    const auto key = makeConstraintKey(constraints[index].idA, constraints[index].idB);
    lookupMap.erase(key);
    // move last value into place of erased value:
    if (index < constraints.size() - 1) {
        auto& last = constraints[constraints.size() - 1];
        constraints[index] = last;
        auto keyLast = makeConstraintKey(last.idA, last.idB);
        lookupMap[keyLast] = index;
    }
    constraints.pop_back();
}

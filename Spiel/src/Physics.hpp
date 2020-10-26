#pragma once

#include <vector>
#include <array>

#include "robin_hood.h"

#include "BaseTypes.hpp"
#include "Vec2.hpp"
#include "PhysicsTypes.hpp"
#include "collision_detection.hpp"
#include "EntityComponentManager.hpp"


inline uint64_t entPairToKey(EntityHandle a, EntityHandle b)
{
	return (uint64_t)a.index << 32 | (uint64_t)b.index;
}
struct PhysicsCollisionData {
	uint32_t entAVersion;
	uint32_t entBVersion;
	float accImpulse{ 0 };
	float accTangentialImpulse{ 0 };
	bool alive{ true };
};

struct CollisionPoint {
	Vec2 position{ 0, 0 };
	Vec2 normal{ 1,0 };
	float massNormal = 0;
	float massTangent = 0;
	// accumulated data:
	float accPn = 0;	// accumulated impulse to normal
	float accPt = 0;	// accumulated impulse to tangent
};

struct CollisionConstraint {
	// meta:
	EntityHandle idA{ 0, 0 };
	EntityHandle idB{ 0, 0 };
	bool updated{ true };
	// collision info:
	CollisionPoint collisionPoints[2] = { CollisionPoint(), CollisionPoint() };
	int collisionPointNum = 1;
	float clippingDist = 0;
	// precomputed data:
	float friction = 0;
	// i dont fucking know: TODO
	float bias = 0;
};

/*
	the higher the priority, the lower the pushout
	priority ranges from 0 to 2, where 2 is no pushout and 0 is whole pushout
*/
Vec2 calcPosChange(float surfAreaA, Vec2 velA, float surfAreaB, Vec2 velB, float const dist, Vec2 const& primCollNormal, bool otherDynamic, float distribution);

inline float calcMomentOfIntertia(float mass, Vec2 size) {
	return mass * (size.x * size.x + size.y * size.y) / 12.0f;
}

std::pair<std::pair<Vec2, float>, std::pair< Vec2, float>> impulseResolution(Vec2 const& posA, Vec2 const& velA, float const anglVelA, float const massA, float const inertiaA,
	Vec2 const& posB, Vec2 const velB, float const anglVelB, float const massB, float const inertiaB, Vec2 const& cNV, Vec2 const& collPos, float e, float f);
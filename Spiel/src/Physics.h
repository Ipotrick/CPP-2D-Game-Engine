#pragma once

#include <vector>
#include <array>

#include "BaseTypes.h"
#include "Vec2.h"
#include "PhysicsTypes.h"

Vec2 calcPosChange(float surfAreaA, Vec2 velA, float surfAreaB, Vec2 velB, float const dist, Vec2 const& primCollNormal, bool otherDynamic);

inline float calcMomentOfIntertia(float mass, Vec2 size) {
	return mass * (size.x * size.x + size.y * size.y) / 12.0f;
}

std::pair<std::pair<Vec2, float>, std::pair< Vec2, float>> impulseResolution(Vec2 const& posA, Vec2 const& velA, float const anglVelA, float const massA, float const inertiaA,
	Vec2 const& posB, Vec2 const velB, float const anglVelB, float const massB, float const inertiaB, Vec2 const& cNV, Vec2 const& collPos, float e, float f);
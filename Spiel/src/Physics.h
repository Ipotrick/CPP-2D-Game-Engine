#pragma once

#include <vector>
#include <array>

#include "glmath.h"
#include "BaseTypes.h"
#include "PhysicsTypes.h"

inline float calcMomentOfIntertia(float mass, vec2 size) {
	return mass * std::max(size.x, size.y) * std::max(size.x, size.y) / 12.0f * 2;
}

inline vec2 boundsSize(Form form, vec2 size, float rotation = 0.0f) {
	if (form == Form::CIRCLE) {
		return size;
	}
	else {
		vec2 maxPos{ 0,0 }; vec2 minPos{ 0,0 };
		vec2 point1 = rotate({ size.x * -0.5f, size.y * -0.5f }, rotation);
		vec2 point2 = rotate90(point1);
		vec2 point3 = rotate90(point2);
		vec2 point4 = rotate90(point3);

		maxPos = max(max(point1, point2), max(point3, point4));
		minPos = min(min(point1, point2), min(point3, point4));

		return maxPos - minPos;
	}
}

__forceinline float dynamicCollision3(float v1, float m1, float v2, float m2, float e) {
	return (e * m2 * (v2 - v1) + m1 * v1 + m2 * v2) / (m1 + m2);
}

inline vec2 dynamicCollision2d3(vec2 const& v1, float const& m1, vec2 const& v2, float const& m2, vec2 const& cNV, float e) {
	float v1Coll = dot(v1, cNV);
	float v2Coll = dot(v2, cNV);
	if (v2Coll - v1Coll > 0.0f) {
		return (dynamicCollision3(v1Coll, m1, v2Coll, m2, e) - v1Coll) * cNV;
	}
	else {
		return 0;
	}
}

inline std::pair<std::pair<vec2, float>, std::pair< vec2, float>> dynamicCollision2d5(	vec2 const& posA, vec2 const& velA, float const anglVelA, float const massA, float const inertiaA, 
																						vec2 const& posB, vec2 const velB, float const anglVelB, float const massB, float const inertiaB, vec2 const& cNV, vec2 const& collPos, float e) {
	vec2 rAP = collPos - posA;
	vec2 rBP = collPos - posB;

	//speed the Collidables have at the specifiy collision point
	vec2 va = velA + rotate90(rAP) * anglVelA / RAD;
	vec2 vb = velB + rotate90(rAP) * anglVelB / RAD;
	float vAB_collDir = dot(va - vb, cNV);

	float j = (-(1.0f + e) * vAB_collDir) /
		(dot(cNV, (cNV * (1 / massA + 1 / massB))) + powf(cross(rAP, cNV), 2) / inertiaA + powf(cross(rBP, cNV), 2) / inertiaB);
	j = std::max(0.0f, j);	// j < 0 => they are not going into each other => no coll response
	return { {j / massA * cNV, cross(rAP, j * cNV) / inertiaA * RAD}, {-j / massB * cNV,-cross(rBP, j * cNV) / inertiaB * RAD} };
}

__forceinline float circleDist(vec2 const pos1, float rad1, vec2 const pos2, float rad2)
{
	float distCaer = distance(pos1, pos2);
	return distCaer - (rad1 + rad2);
}

__forceinline bool isIntervalCenterSmaller(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	auto centerFirst = (maxFirst - minFirst) * 0.5f + minFirst;
	auto centerSecond = (maxSecond - minSecond) * 0.5f + minSecond;
	return (centerFirst < centerSecond);
}

__forceinline bool doIntervalsOverlap(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	return !(minFirst > maxSecond || minSecond > maxFirst);
}

__forceinline float clippingDist(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	if (minFirst > maxSecond || minSecond > maxFirst) {
		return 0.0f;
	}
	else {
		std::array<float, 2> distances{ fabs(minFirst - maxSecond), fabs(maxFirst - minSecond) };
		return *std::min_element(distances.begin(), distances.end());
	}
}

// primCollNormal is allways FROM other TO coll, dist is the dist TO push out, so dist > 0!
vec2 calcPosChange(CollidableAdapter const* coll, CollidableAdapter const* other, float const dist, vec2 const& primCollNormal);

CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other, bool bothSolid);

std::tuple<bool, float, float, vec2> partialSATCollision(CollidableAdapter const* coll, CollidableAdapter const* other);

CollisionTestResult rectangleRectangleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other, bool bothSolid);

CollisionTestResult checkCircleRectangleCollision(CollidableAdapter const* circle, CollidableAdapter const* rect, bool bothSolid, bool isCirclePrimary);

inline bool isOverlappingAABB(CollidableAdapter const* a, CollidableAdapter const* b) {
	auto a_AABB = boundsSize(a->getForm(), a->getSize(), a->getRota());
	auto b_AABB = boundsSize(b->getForm(), b->getSize(), b->getRota());
	return fabs(b->getPos().x - a->getPos().x) <= fabs(b_AABB.x + a_AABB.x) * 0.5f 
		&& fabs(b->getPos().y - a->getPos().y) <= fabs(b_AABB.y + a_AABB.y) * 0.5f;
}

CollisionTestResult checkForCollision(CollidableAdapter const* coll_, CollidableAdapter const* other_);
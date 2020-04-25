#pragma once

#include <vector>
#include <array>

#include "BaseTypes.h"
#include "Vec2.h"
#include "PhysicsTypes.h"

inline Vec2 calcPosChange(float surfAreaA, Vec2 velA, float surfAreaB, Vec2 velB, float const dist, Vec2 const& primCollNormal, bool otherDynamic) {
	if (otherDynamic) {
		float bothAreas = surfAreaA + surfAreaB;
		float bPart = surfAreaB / bothAreas;
		float collDirV1 = dot(velA, primCollNormal);
		float collDirV2 = dot(velB, primCollNormal);
		if (collDirV1 - collDirV2 > 0.0f && bPart < 0.75f && bPart > 0.25f) {
			//they move into each other
			if (collDirV2 < 0) {
				//coll moved into other
				return dist * primCollNormal * 0.9f + primCollNormal * 0.0001f;
			}
			else {
				//other moved into coll
				return dist * primCollNormal * 0.1f + primCollNormal * 0.0001f;
			}
		}
		else {
			return dist * primCollNormal * bPart + primCollNormal * 0.0001f;
		}
	}
	else {
		return dist * primCollNormal + primCollNormal * 0.0001f;
	}
}

inline float calcMomentOfIntertia(float mass, Vec2 size) {
	return mass * (size.x * size.x + size.y * size.y) / 12.0f;
}

__forceinline float dynamicCollision3(float v1, float m1, float v2, float m2, float e) {
	return (e * m2 * (v2 - v1) + m1 * v1 + m2 * v2) / (m1 + m2);
}

inline Vec2 dynamicCollision2d3(Vec2 const& v1, float const& m1, Vec2 const& v2, float const& m2, Vec2 const& cNV, float e) {
	float v1Coll = dot(v1, cNV);
	float v2Coll = dot(v2, cNV);
	if (v2Coll - v1Coll > 0.0f) {
		return (dynamicCollision3(v1Coll, m1, v2Coll, m2, e) - v1Coll) * cNV;
	}
	else {
		return Vec2(0,0);
	}
}

inline std::pair<std::pair<Vec2, float>, std::pair< Vec2, float>> dynamicCollision2d5(	Vec2 const& posA, Vec2 const& velA, float const anglVelA, float const massA, float const inertiaA, 
																						Vec2 const& posB, Vec2 const velB, float const anglVelB, float const massB, float const inertiaB, Vec2 const& cNV, Vec2 const& collPos, float e) {
	Vec2 rAP = collPos - posA;
	Vec2 rBP = collPos - posB;

	//speed the Collidables have at the specifiy collision point
	Vec2 va = velA + rotate90(rAP) * anglVelA / RAD;
	Vec2 vb = velB + rotate90(rAP) * anglVelB / RAD;
	float vAB_collDir = dot(va - vb, cNV);

	Vec2 rAPTangent = rotate90(rAP);

	float j = (-(1.0f + e) * vAB_collDir) /
		(dot(cNV, (cNV * (1 / massA + 1 / massB))) + powf(cross(rAP, cNV), 2) / inertiaA + powf(cross(rBP, cNV), 2) / inertiaB);
	j = std::max(0.0f, j);	// j < 0 => they are not going into each other => no coll response
	return { {j / massA * cNV, cross(rAP, j * cNV) / inertiaA * RAD}, {-j / massB * cNV,-cross(rBP, j * cNV) / inertiaB * RAD} };
}

inline std::pair<std::pair<Vec2, float>, std::pair< Vec2, float>> dynamicCollision2d6(Vec2 const& posA, Vec2 const& velA, float const anglVelA, float const massA, float const inertiaA,
	Vec2 const& posB, Vec2 const velB, float const anglVelB, float const massB, float const inertiaB, Vec2 const& cNV, Vec2 const& collPos, float e, float f) {

	Vec2 vA_relative_to_vB = velA - velB;

	Vec2 rAP = collPos - posA;
	Vec2 rBP = collPos - posB;

	Vec2 vAP = velA + cross(rAP, anglVelA / RAD);
	Vec2 vBP = velB + cross(rBP, anglVelB / RAD);

	Vec2 vAP_relative_to_vBP = vAP - vBP;

	float contactVel = dot(vAP_relative_to_vBP, cNV);
	if (contactVel > 0) {	// keine kollision wenn sich körper auseinander bewegen
		return { {Vec2(0.0f,0.0f), 0.0f}, {Vec2(0.0f,0.0f), 0.0f} };
	}

	float j = (-(1.0f + e) * contactVel) / 
		(1.0f / massA + 1.0f / massB 
			+ (cross(rAP, cNV) * cross(rAP, cNV) / inertiaA)
			+ (cross(rBP, cNV) * cross(rBP, cNV) / inertiaB));

	Vec2 tNV = rotate90(cNV);
	float tangentContactVel = dot(vAP_relative_to_vBP, tNV);

	float frictionJ = 0;/* (-(1 + f) * tangentContactVel) /
		(1 / massA + 1 / massB
			+ (cross(rAP, tNV) * cross(rAP, tNV) / inertiaA)
			+ (cross(rBP, tNV) * cross(rBP, tNV) / inertiaB));*/

	return {
		{ cNV *  j / massA + tNV * frictionJ / massA, 
		cross(rAP, cNV) *  j / inertiaA * RAD + cross(rAP, tNV) * frictionJ / inertiaA * RAD },
		{ cNV * -j / massB + tNV *  -frictionJ / massA,
		cross(rBP, cNV) * -j / inertiaB * RAD + cross(rBP, tNV) * -frictionJ / inertiaB * RAD }
	};
}
#include "Physics.h"

Vec2 calcPosChange(float surfAreaA, Vec2 velA, float surfAreaB, Vec2 velB, float const dist, Vec2 const& primCollNormal, bool otherDynamic, float priority) {
	if (otherDynamic) {
		float bothAreas = surfAreaA + surfAreaB;
		float bPart = surfAreaB / bothAreas;
		float collDirV1 = dot(velA, primCollNormal);
		float collDirV2 = dot(velB, primCollNormal);
		if (collDirV1 - collDirV2 > 0.0f && bPart < 0.95f && bPart > 0.05f && Physics::directionalPositionCorrection) {
			//they move into each other
			if (collDirV2 < 0) {
				//coll moved into other
				return dist * primCollNormal *  (2.0f - priority) * Physics::directionalCorrectionFactor + primCollNormal * Physics::pushout;
			}
			else {
				//other moved into coll
				return dist * primCollNormal * (2.0f- priority) * (1.0f - Physics::directionalCorrectionFactor) + primCollNormal * Physics::pushout;
			}
		}
		else {
			return dist * primCollNormal * bPart * (2.0f - priority) + primCollNormal * Physics::pushout;
		}
	}
	else {
		return dist * primCollNormal + primCollNormal * Physics::pushout;
	}
}

std::pair<std::pair<Vec2, float>, std::pair< Vec2, float>> impulseResolution(Vec2 const& posA, Vec2 const& velA, float const anglVelA, float const massA, float const inertiaA,
	Vec2 const& posB, Vec2 const velB, float const anglVelB, float const massB, float const inertiaB, Vec2 const& cNV, Vec2 const& collPos, float e, float f) {

	Vec2 rAP = collPos - posA;
	Vec2 rBP = collPos - posB;

	Vec2 vAP = velA + rotate<90>(rAP) * anglVelA / RAD;
	Vec2 vBP = velB + rotate<90>(rBP) * anglVelB / RAD;

	Vec2 vAP_relative_to_vBP = vAP - vBP;

	float contactVel = dot(vAP_relative_to_vBP, cNV);
	if (contactVel > 0) {	// keine kollision wenn sich körper auseinander bewegen
		return { {Vec2(0.0f,0.0f), 0.0f}, {Vec2(0.0f,0.0f), 0.0f} };
	}

	float impulse = (-(1.0f + e) * contactVel) /
		(1.0f / massA + 1.0f / massB
			+ (cross(rAP, cNV) * cross(rAP, cNV) / inertiaA)
			+ (cross(rBP, cNV) * cross(rBP, cNV) / inertiaB));

	Vec2 tNV = rotate<90>(cNV);
	float tangentContactVel = dot(vAP_relative_to_vBP, tNV);

	float frictionImpulse = f * (-tangentContactVel) /
		(1 / massA + 1 / massB
			+ (cross(rAP, tNV) * cross(rAP, tNV) / inertiaA)
			+ (cross(rBP, tNV) * cross(rBP, tNV) / inertiaB));

	return {
		{ cNV * impulse / massA + tNV * frictionImpulse / massA,
		cross(rAP, cNV) * impulse / inertiaA * RAD + cross(rAP, tNV) * frictionImpulse / inertiaA * RAD },
		{ cNV * -impulse / massB + tNV * -frictionImpulse / massA,
		cross(rBP, cNV) * -impulse / inertiaB * RAD + cross(rBP, tNV) * -frictionImpulse / inertiaB * RAD }
	};
}
#pragma once

#include <array>
#include <optional>

#include "../../engine/types/BaseTypes.hpp"
#include "../../engine/math/Vec2.hpp"
#include "../../engine/entity/EntityTypes.hpp"
#include "../../engine/rendering/RenderTypes.hpp"
#include "CollisionUniform.hpp"

struct CollPoint {
	CollPoint(Vec2 p, Vec2 n, float c)
		:pos{ p }, norm{ n }, clip{ c }
	{}
	Vec2 pos;
	Vec2 norm;
	float clip;
};

struct CollidableAdapter {
	CollidableAdapter(Vec2 const& pos_, Vec2 const& size_, Form const& form_, RotaVec2 rotationVec_) :
		position{ pos_ },
		size{ size_ },
		rotationVec{ rotationVec_ },
		form{ form_ }
	{}
	CollidableAdapter() :
		position{ 0,0 },
		size{ 0,0 },
		rotationVec{ 0,0 },
		form{ Form::Circle }
	{}
	inline CollidableAdapter& operator=(CollidableAdapter const& rhs) = default;
	inline float getSurfaceArea() const 
	{ 
		return size.x * size.y;
	}
	Vec2 position;
	Vec2 size;
	RotaVec2 rotationVec;
	Form form;
};

struct CollisionTestResult {
	Vec2 collisionNormal;
	Vec2 collisionPos;
	Vec2 collisionPos2;
	float clippingDist;
	int collisionCount;

	CollisionTestResult() 
		: collisionPos{ 0, 0 }, collisionCount{ 0 }, clippingDist{ 0.0f }, collisionNormal{ 1,0 } 
	{}
};

struct CollisionInfo {
	EntityHandleIndex indexA;
	EntityHandleIndex indexB;
	Vec2 normal[2];
	Vec2 position[2];
	int collisionPointNum;
	float clippingDist;

	CollisionInfo(EntityHandleIndex idA, EntityHandleIndex idB, float clippingDist, Vec2 collisionNormal, Vec2 collisionNormal2, Vec2 collisionPos, Vec2 collisionPos2, int collCount)
		: indexA{ idA }, indexB{ idB }, clippingDist{ clippingDist }, normal{ collisionNormal, collisionNormal2 }, position{ collisionPos, collisionPos2 }, collisionPointNum{ collCount }
	{}
};

struct CollisionResponse { 
	Vec2 posChange = Vec2(0,0);
};

__forceinline float circleDist(Vec2 const pos1, float rad1, Vec2 const pos2, float rad2)
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

struct SATTestResult {
	bool collisionPointCount{ true };
	float minClippingDist{ 1000000000000000.0f };
	Vec2 cornerPosOfMinClippingDist{ 0,0 };
	Vec2 pos1;
	Vec2 pos2;
	Vec2 collisionNormalOfMinClippingDist{ 1,0 };
};

SATTestResult partialSATTest(CollidableAdapter const& coll, CollidableAdapter const& other);

CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const& coll, CollidableAdapter const& other, bool bothSolid);

CollisionTestResult rectangleRectangleCollisionCheck2(CollidableAdapter const& coll, CollidableAdapter const& other);

CollisionTestResult rectangleRectangleCollisionCheck3(CollidableAdapter const& coll, CollidableAdapter const& other);

CollisionTestResult checkCircleRectangleCollision(CollidableAdapter const& circle, CollidableAdapter const& rect, bool bothSolid, bool isCirclePrimary);

inline bool isOverlappingAABB(Vec2 const a_pos, Vec2 const a_AABB, Vec2 const b_pos, Vec2 const b_AABB) 
{
	return (std::abs(b_pos.x - a_pos.x) < std::abs(b_AABB.x + a_AABB.x) * 0.5f)
		& (std::abs(b_pos.y - a_pos.y) < std::abs(b_AABB.y + a_AABB.y) * 0.5f);
}


CollisionTestResult collisionTest(CollidableAdapter const& coll_, CollidableAdapter const& other_);

CollisionTestResult collisionTest(CollidableAdapter const& coll_, CollidableAdapter const& other_);


inline void generateCollisionInfos2(
	CollisionSECM manager,
	std::vector<CollisionInfo>& collisionInfos,
	std::vector<Vec2> const& aabbCache,
	const std::vector<EntityHandleIndex>& nearCollidablesBuffer,
	const EntityHandleIndex me,
	const Transform& baseColl,
	const Collider& colliderColl,
	const Vec2 aabbMe,
	std::vector<CollPoint>& collisionVertices)
{
	const CollidableAdapter collAdapter = CollidableAdapter(
		baseColl.position,
		colliderColl.size,
		colliderColl.form,
		baseColl.rotaVec);
	for (const auto otherEnt : nearCollidablesBuffer) {
		if (me != otherEnt) { //do not check against self
			const auto& baseOther = manager.getComp<Transform>(otherEnt);
			const auto& colliderOther = manager.getComp<Collider>(otherEnt);
			if (!(
				colliderColl.ignoreGroupMask
				& colliderOther.groupMask)
				) {
				if (isOverlappingAABB(baseColl.position, aabbMe, baseOther.position, aabbCache.at(otherEnt))) {
					if (colliderColl.extraColliders.empty() & colliderOther.extraColliders.empty()) {
						CollidableAdapter otherAdapter(
							baseOther.position,
							colliderOther.size,
							colliderOther.form,
							baseOther.rotaVec);

						const auto newTestResult = collisionTest(collAdapter, otherAdapter);
						if (newTestResult.collisionCount > 0) {
							collisionInfos.push_back(CollisionInfo(me, otherEnt, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionNormal, newTestResult.collisionPos, newTestResult.collisionPos2, newTestResult.collisionCount));
						}
					}
					else {
						collisionVertices.clear();

						CollidableAdapter otherAdapter(baseOther.position, colliderOther.size, colliderOther.form, baseOther.rotaVec);
						auto testForCollision = [&](CollidableAdapter collAdapter, CollidableAdapter otherAdapter) {
							const auto newTestResult = collisionTest(collAdapter, otherAdapter);
							if (newTestResult.collisionCount >= 1)
								collisionVertices.push_back({ newTestResult.collisionPos, newTestResult.collisionNormal, newTestResult.clippingDist });
							if (newTestResult.collisionCount == 2)
								collisionVertices.push_back({ newTestResult.collisionPos2, newTestResult.collisionNormal, newTestResult.clippingDist });
						};
						testForCollision(collAdapter, otherAdapter);
						for (auto& oc : colliderOther.extraColliders) {
							CollidableAdapter otherAdapter(baseOther.position + rotate(oc.relativePos, baseOther.rotaVec), oc.size, oc.form, baseOther.rotaVec * oc.relativeRota);
							testForCollision(collAdapter, otherAdapter);
						}
						for (auto& cc : colliderColl.extraColliders) {
							const CollidableAdapter collAdapter = CollidableAdapter(baseColl.position + rotate(cc.relativePos, baseColl.rotaVec), cc.size, cc.form, baseColl.rotaVec * cc.relativeRota);
							testForCollision(collAdapter, otherAdapter);
							for (auto& oc : colliderOther.extraColliders) {
								CollidableAdapter otherAdapter(baseOther.position + rotate(oc.relativePos, baseOther.rotaVec), oc.size, oc.form, baseOther.rotaVec * oc.relativeRota);
								testForCollision(collAdapter, otherAdapter);
							}
						}
						if (collisionVertices.size() > 1) {
							Vec2 minV{ FLT_MIN, FLT_MIN };
							Vec2 maxV{ FLT_MAX, FLT_MAX };
							for (const auto& v : collisionVertices) {
								minV = min(v.pos, minV);
								maxV = max(v.pos, maxV);
							}
							Vec2 midPoint = minV + maxV * 0.5f;
							int vertex1 = 0;
							float mostMiddleDistance = distance(collisionVertices[0].pos, midPoint);
							for (int i = 1; i < collisionVertices.size(); i++) {
								float newMiddleDistance = distance(collisionVertices[i].pos, midPoint);
								if (newMiddleDistance > mostMiddleDistance) {
									vertex1 = i;
									mostMiddleDistance = newMiddleDistance;
								}
							}

							int vertex2 = vertex1;
							float distVertex = 0.0f;
							for (int i = 0; i < collisionVertices.size(); i++) {
								float newDist = distance(collisionVertices[i].pos, collisionVertices[vertex1].pos);
								if (newDist > distVertex) {
									vertex2 = i;
									distVertex = newDist;
								}
							}

							// point one is allways on the left side, point two is allways on the right
							Vec2 centerTangent = rotate<90>(normalize(baseOther.position - baseColl.position));	// dot < 0 = left side dot > 0 = right side
							Vec2 relPosV1 = collisionVertices[vertex1].pos - baseColl.position;
							Vec2 relPosV2 = collisionVertices[vertex2].pos - baseColl.position;
							// when vertex1 is more right than vertex2 we swap them
							if (dot(relPosV1, centerTangent) > dot(relPosV2, centerTangent)) {
								std::swap(vertex1, vertex2);
							}

							float clip = (collisionVertices[vertex1].clip + collisionVertices[vertex2].clip) * 0.5f;
							collisionInfos.push_back(CollisionInfo(me, otherEnt, clip, collisionVertices[vertex1].norm, collisionVertices[vertex2].norm, collisionVertices[vertex1].pos, collisionVertices[vertex2].pos, 2));
						}
						else if (collisionVertices.size() == 1) {
							collisionInfos.push_back(CollisionInfo(me, otherEnt, collisionVertices[0].clip, collisionVertices[0].norm, collisionVertices[0].norm, collisionVertices[0].pos, collisionVertices[0].pos, 1));
						}
					}
				}
			}
		}
	}
}
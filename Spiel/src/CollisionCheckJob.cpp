#include "CollisionCheckJob.hpp"


void CollisionCheckJob::collisionFunction(
	const Entity collID,
	const Quadtree& qtree,
	std::vector<Entity>& nearCollidablesBuffer,
	std::vector<CollisionInfo>& collisionInfos)
{
	const auto colliderColl = manager.getComp<Collider>(collID);
	const auto baseColl = manager.getComp<Base>(collID);

	if (!colliderColl.sleeping) {
		const CollidableAdapter collAdapter = CollidableAdapter(
			baseColl.position,
			colliderColl.size,
			colliderColl.form,
			baseColl.rotaVec);

		nearCollidablesBuffer.clear();
		qtree.querry(&nearCollidablesBuffer, baseColl.position, aabbCache.at(collID));

		for (const auto otherID : nearCollidablesBuffer) {
			if (collID != otherID) { //do not check against self
				const auto baseOther = manager.getComp<Base>(otherID);
				const auto colliderOther = manager.getComp<Collider>(otherID);
				if (!(colliderColl.ignoreGroupMask & colliderOther.groupMask)) {
					if (colliderColl.extraColliders.empty() & colliderOther.extraColliders.empty()) {
						CollidableAdapter otherAdapter(
							baseOther.position,
							colliderOther.size,
							colliderOther.form,
							baseOther.rotaVec);

						const auto newTestResult = collisionTestCachedAABB(collAdapter, otherAdapter, aabbCache.at(collID), aabbCache.at(otherID));
						if (newTestResult.collisionCount > 0) {
							collisionInfos.push_back(CollisionInfo(collID, otherID, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionNormal, newTestResult.collisionPos, newTestResult.collisionPos2, newTestResult.collisionCount));
						}
					}
					else {
						collisionVertices.clear();

						CollidableAdapter otherAdapter(baseOther.position, colliderOther.size, colliderOther.form, baseOther.rotaVec);
						auto testForCollision = [&](CollidableAdapter collAdapter, CollidableAdapter otherAdapter) {
							const auto newTestResult = collisionTestCachedAABB(collAdapter, otherAdapter, aabbCache.at(collID), aabbCache.at(otherID));
							if (newTestResult.collisionCount >= 1) 
								collisionVertices.push_back({ newTestResult.collisionPos, newTestResult.collisionNormal, newTestResult.clippingDist });
							if (newTestResult.collisionCount == 2) 
								collisionVertices.push_back({ newTestResult.collisionPos2, newTestResult.collisionNormal, newTestResult.clippingDist });
						};
						testForCollision(collAdapter, otherAdapter);
						for (auto oc : colliderOther.extraColliders) {
							CollidableAdapter otherAdapter( baseOther.position + rotate(oc.relativePos, baseOther.rotaVec), oc.size, oc.form, baseOther.rotaVec * oc.relativeRota);
							testForCollision(collAdapter, otherAdapter);
						}
						for (auto cc : colliderColl.extraColliders) {
							const CollidableAdapter collAdapter = CollidableAdapter( baseColl.position + rotate(cc.relativePos, baseColl.rotaVec), cc.size, cc.form, baseColl.rotaVec * cc.relativeRota);
							testForCollision(collAdapter, otherAdapter);
							for (auto oc : colliderOther.extraColliders) {
								CollidableAdapter otherAdapter( baseOther.position + rotate(oc.relativePos, baseOther.rotaVec), oc.size, oc.form, baseOther.rotaVec * oc.relativeRota);
								testForCollision(collAdapter, otherAdapter);
							}
						}
						if (collisionVertices.size() > 1) {
							Vec2 minV{ FLT_MIN, FLT_MIN };
							Vec2 maxV{ FLT_MAX, FLT_MAX };
							for (auto v : collisionVertices) {
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
							Vec2 centerTangent = rotate<270>(normalize(baseOther.position - baseColl.position));	// dot > 0 = left side dot < 0 = right side
							Vec2 relPosV1 = collisionVertices[vertex1].pos - baseColl.position;
							Vec2 relPosV2 = collisionVertices[vertex2].pos - baseColl.position;
							// when vertex1 is more right than vertex2 we swap them
							if (dot(relPosV1, centerTangent) < dot(relPosV1, centerTangent)) {
								std::swap(vertex1, vertex2);
							}

							float clip = (collisionVertices[vertex1].clip + collisionVertices[vertex2].clip) * 0.5f;
							collisionInfos.push_back(CollisionInfo(collID, otherID, clip, collisionVertices[vertex1].norm, collisionVertices[vertex2].norm, collisionVertices[vertex1].pos, collisionVertices[vertex2].pos, 2));
						}
						else if (collisionVertices.size() == 1) {
							collisionInfos.push_back(CollisionInfo(collID, otherID, collisionVertices[0].clip, collisionVertices[0].norm, collisionVertices[0].norm, collisionVertices[0].pos, collisionVertices[0].pos, 1));
						}
					}
				}
			}
		}
	}
}
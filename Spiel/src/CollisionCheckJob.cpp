#include "CollisionCheckJob.hpp"


void CollisionCheckJob::collisionFunction(
	const EntityHandleIndex collID,
	const Quadtree& qtree,
	std::vector<EntityHandleIndex>& nearCollidablesBuffer,
	std::vector<CollisionInfo>& collisionInfos)
{
	const auto& baseColl = manager.getComp<Transform>(collID);
	const auto& colliderColl = manager.getComp<Collider>(collID);

	if (!colliderColl.sleeping) {

		nearCollidablesBuffer.clear();
		qtree.querry(nearCollidablesBuffer, baseColl.position, aabbCache.at(collID));

		generateCollisionInfos(manager, collisionInfos, aabbCache, nearCollidablesBuffer, collID, baseColl, colliderColl, aabbCache.at(collID), collisionVertices);
	}
}
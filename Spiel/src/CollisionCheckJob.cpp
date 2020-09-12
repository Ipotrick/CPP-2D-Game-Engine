#include "CollisionCheckJob.hpp"


void CollisionCheckJob::collisionFunction(
	const Entity collID,
	const Quadtree& qtree,
	std::vector<Entity>& nearCollidablesBuffer,
	std::vector<CollisionInfo>& collisionInfos)
{
	const auto& baseColl = manager.getComp<Base>(collID);
	const auto& colliderColl = manager.getComp<Collider>(collID);

	if (!colliderColl.sleeping) {

		nearCollidablesBuffer.clear();
		qtree.querry(nearCollidablesBuffer, baseColl.position, aabbCache.at(collID));

		generateCollisionInfos(manager, collisionInfos, aabbCache, nearCollidablesBuffer, collID, baseColl, colliderColl, aabbCache.at(collID), collisionVertices);
	}
}
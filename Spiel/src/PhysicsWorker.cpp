#include "PhysicsWorker.h"

void PhysicsWorker::operator()()
{
	std::vector<Collidable*> nearCollidables;	//reuse heap memory for all dyn collidable collisions
	for (int i = begin; i < end; i++) {
		auto& coll = dynCollidables.at(i);
		nearCollidables.clear();

		qtree.querry(nearCollidables, coll->getPos(), coll->getBoundsSize());

		for (auto& other : nearCollidables) {
			auto newResponse = checkForCollision(coll, other);
			(collisionResponses)[i] = (collisionResponses)[i] + newResponse;
			if (newResponse.collided) {
				collisionInfos.push_back(CollisionInfo(coll->getId(), other->getId()));
			}
		}
	}
}

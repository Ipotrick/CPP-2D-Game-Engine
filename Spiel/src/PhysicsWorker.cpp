#include "PhysicsWorker.h"

void PhysicsWorker::operator()()
{
	while (true) {
		{
			std::unique_lock<std::mutex> switch_lock(syncData->mut);
			syncData->go.at(physicsData->id) = false;
			syncData->cond.notify_all();
			syncData->cond.wait(switch_lock, [&]() { return syncData->go.at(physicsData->id) ==  true; });	//wait for engine to give go sign
			if (syncData->run == false) break;
		}

		auto& begin = physicsData->begin;
		auto& end = physicsData->end;
		auto& dynCollidables = physicsData->dynCollidables;
		auto& qtree = physicsData->qtree;
		auto& collisionResponses = physicsData->collisionResponses;
		auto& collisionInfos = physicsData->collisionInfos;
		
		std::vector<Collidable*> nearCollidables;	//reuse heap memory for all dyn collidable collisions
		for (int i = begin; i < end; i++) {
			auto& coll = dynCollidables->at(i);
			nearCollidables.clear();

			qtree->querry(nearCollidables, coll->getPos(), coll->getBoundsSize());

			for (auto& other : nearCollidables) {
				auto newResponse = checkForCollision(coll, other);
				(*collisionResponses)[i] = (*collisionResponses)[i] + newResponse;
				
				if (newResponse.collided) {
					collisionInfos->push_back(CollisionInfo(coll->getId(), other->getId()));
				}
			}
		}
	}
}

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

			//check for collisions and save the changes in velocity and position these cause
			for (auto& other : nearCollidables) {
				auto newResponse = checkForCollision(coll, other);
				
				if (newResponse.collided) {
					// set weighted average of position changes 
					auto bothPosChangeLengths = norm((*collisionResponses)[i].posChange) + norm(newResponse.posChange);
					if (bothPosChangeLengths > 0) {
						(*collisionResponses)[i].posChange = norm((*collisionResponses)[i].posChange) / bothPosChangeLengths * (*collisionResponses)[i].posChange
							+ norm(newResponse.posChange) / bothPosChangeLengths * newResponse.posChange;
					}

					(*collisionResponses)[i].velChange = (*collisionResponses)[i].velChange + newResponse.velChange;

					(*collisionResponses)[i].collided = true;
				}
				if (newResponse.collided) {
					collisionInfos->push_back(CollisionInfo(coll->getId(), other->getId(), newResponse.clippingDist));
				}
			}

			//add the velocity change through acceleration
			(*collisionResponses)[i].velChange += coll->acceleration * physicsData->deltaTime;
		}
	}
}

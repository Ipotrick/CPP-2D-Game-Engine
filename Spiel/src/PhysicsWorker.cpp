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
		
		std::vector<std::pair<uint32_t, Collidable*>> nearCollidables;	//reuse heap memory for all dyn collidable collisions
		for (int i = begin; i < end; i++) {
			auto& coll = dynCollidables->at(i);
			nearCollidables.clear();

			qtree->querry(nearCollidables, coll.second->getPos(), coll.second->getBoundsSize());

			//check for collisions and save the changes in velocity and position these cause
			for (auto& other : nearCollidables) {
				auto newResponse = checkForCollision(coll.second, other.second);
				
				if (newResponse.collided) {
					// set weighted average of position changes 
					auto bothPosChangeLengths = norm((*collisionResponses)[i].posChange) + norm(newResponse.posChange);
					if (bothPosChangeLengths > 0) {
						(*collisionResponses)[i].posChange = norm((*collisionResponses)[i].posChange) / bothPosChangeLengths * (*collisionResponses)[i].posChange
							+ norm(newResponse.posChange) / bothPosChangeLengths * newResponse.posChange;
					}

					(*collisionResponses)[i].velChange = (*collisionResponses)[i].velChange + newResponse.velChange;

					(*collisionResponses)[i].collided = true;
					collisionInfos->push_back(CollisionInfo(coll.first, other.first, newResponse.clippingDist));
				}
			}

			//add the velocity change through acceleration
			(*collisionResponses)[i].velChange += coll.second->acceleration * physicsData->deltaTime;
			//restrain acelleration change
			float absVelBefore = norm(coll.second->getVel());
			float absVelAfter = norm(coll.second->getVel() + (*collisionResponses)[i].velChange);
			float difference = absVelAfter - absVelBefore;
			if (difference > Physics::maxPosAbsVelChange) {	//if maxVelChange is too high
				float correcturFactor = Physics::maxPosAbsVelChange / difference;
				(*collisionResponses)[i].velChange = (*collisionResponses)[i].velChange * correcturFactor;
			}
		}
	}
}

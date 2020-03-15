#include "PhysicsWorker.h"
#include <algorithm>
#include <random>

void PhysicsWorker::operator()()
{
	auto& beginDyn = physicsData->beginDyn;
	auto& endDyn = physicsData->endDyn;
	auto& dynCollidables = physicsData->dynCollidables;
	auto& beginStat = physicsData->beginStat;
	auto& endStat = physicsData->endStat;
	auto& statCollidables = physicsData->statCollidables;
	auto& collisionResponses = physicsData->collisionResponses;
	auto& collisionInfos = physicsData->collisionInfos;
	auto& qtrees = physicsData->qtrees;

	while (true) {
		{
			std::unique_lock<std::mutex> switch_lock(syncData->mut);
			syncData->go.at(physicsData->id) = false;
			syncData->cond.notify_all();
			syncData->cond.wait(switch_lock, [&]() { return syncData->go.at(physicsData->id) ==  true; });	//wait for engine to give go sign
			if (syncData->run == false) break;
		}

		// build qtrees
		for (int i = beginStat; i < endStat; i++) {
			qtrees->at(physicsData->id).insert(statCollidables->at(i));
		}
		for (int i = beginDyn; i < endDyn; i++) {
			qtrees->at(physicsData->id).insert(dynCollidables->at(i));
		}
		

		// re sync with others after inserting
		{
			std::unique_lock<std::mutex> switch_lock(syncData->mut2);
			syncData->insertReady++;
			if (syncData->insertReady == physicsThreadCount) {
				syncData->insertReady = 0;
				syncData->cond2.notify_all();
			}
			else {
				syncData->cond2.wait(switch_lock, [&]() { return syncData->insertReady == 0; });
			}
		}
		
		
		std::vector<std::pair<uint32_t, Collidable*>> nearCollidables;	//reuse heap memory for all dyn collidable collisions
		for (int i = beginDyn; i < endDyn; i++) {
			auto& coll = dynCollidables->at(i);
			nearCollidables.clear();

			for (int i = 0; i < physicsThreadCount; i++) {
				qtrees->at(i).querry(nearCollidables, coll.second->getPos(), coll.second->getBoundsSize());
			}

			// heuristic to reduce bad velocity stacking
			float const smallParticleArea{ 0.15f*0.15f };
			
			unsigned int const maxVelChanges = static_cast<unsigned>(floorf(coll.second->getHitboxSize().x * coll.second->getHitboxSize().y * (1 / smallParticleArea))) + 3;
			unsigned int velChanges{ 0 };

			//check for collisions and save the changes in velocity and position these cause
			for (auto& other : nearCollidables) {
				if (coll.first != other.first) {

					auto newResponse = checkForCollision(coll.second, other.second);

					if (newResponse.collided) {
						// set weighted average of position changes 
						auto bothPosChangeLengths = norm((*collisionResponses)[i].posChange) + norm(newResponse.posChange);
						if (bothPosChangeLengths > 0) {
							(*collisionResponses)[i].posChange = norm((*collisionResponses)[i].posChange) / bothPosChangeLengths * (*collisionResponses)[i].posChange
								+ norm(newResponse.posChange) / bothPosChangeLengths * newResponse.posChange;
						}

						if (norm(newResponse.velChange) > 0.01f) {
							velChanges++;
						}
						if (velChanges < maxVelChanges) {
							(*collisionResponses)[i].velChange = (*collisionResponses)[i].velChange + newResponse.velChange;
						}

						(*collisionResponses)[i].collided = true;
						collisionInfos->push_back(CollisionInfo(coll.first, other.first, newResponse.clippingDist));

					}
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

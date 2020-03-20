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
			(*collisionResponses)[i].posChange = vec2(0,0);		//initialize posChange
			auto& coll = dynCollidables->at(i);
			nearCollidables.clear();

			for (int i = 0; i < physicsThreadCount; i++) {
				qtrees->at(i).querry(nearCollidables, coll.second->getPos(), coll.second->getBoundsSize());
			}

			//check for collisions and save the changes in velocity and position these cause
			for (auto& other : nearCollidables) {
				if (coll.first != other.first) {

					//if (coll.second->isSolid() || other.second->isDynamic()) {	// coll.solid = false && other.dynamic == false : sensors do not check for static entities
						auto newTestResult = checkForCollision(coll.second, other.second);

						if (newTestResult.collided) {
							collisionInfos->push_back(CollisionInfo(coll.first, other.first, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionPos));
							//take average of pushouts with weights
							float weightOld = norm((*collisionResponses)[i].posChange);
							float weightNew = norm(newTestResult.posChange);
							float normalizer = weightOld + weightNew;
							if (normalizer > Physics::nullDelta) {
								(*collisionResponses)[i].posChange = ((*collisionResponses)[i].posChange * weightOld / normalizer + newTestResult.posChange * weightNew / normalizer);
							}
						}
					//}
				}
			}
		}
	}
}

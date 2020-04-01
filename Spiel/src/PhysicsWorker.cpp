#include "PhysicsWorker.h"
#include <algorithm>
#include <random>

void PhysicsWorker::operator()()
{
	auto& world = *physicsPoolData->world;
	auto& beginDyn = physicsData->beginDyn;
	auto& endDyn = physicsData->endDyn;
	auto& dynCollidables = physicsPoolData->dynCollidables;
	auto& beginStat = physicsData->beginStat;
	auto& endStat = physicsData->endStat;
	auto& statCollidables = physicsPoolData->statCollidables;
	auto& collisionResponses = physicsPoolData->collisionResponses;
	auto& collisionInfos = physicsData->collisionInfos;
	auto& qtreesDynamic = physicsPoolData->qtreesDynamic;
	auto& qtreesStatic = physicsPoolData->qtreesStatic;

	while (true) {
		{
			std::unique_lock<std::mutex> switch_lock(syncData->mut);
			syncData->go.at(physicsData->id) = false;
			syncData->cond.notify_all();
			syncData->cond.wait(switch_lock, [&]() { return syncData->go.at(physicsData->id) == true; });	//wait for engine to give go sign
			if (syncData->run == false) break;
		}

		if (physicsPoolData->dynCollidables) {
			// rebuild dyn qtree
			if (physicsPoolData->rebuildDynQuadTrees) {
				for (int i = beginDyn; i < endDyn; i++) {
					if (!dynCollidables->at(i).second.second.particle) {	//never check for collisions against particles
						PosSize aabb(
							dynCollidables->at(i).second.first.position,
							dynCollidables->at(i).second.second.size);
						qtreesDynamic->at(physicsData->id).insert({ dynCollidables->at(i).first, aabb });
					}
				}
			}
			// rebuild stat qtree
			if (physicsPoolData->rebuildStatQuadTrees) {
				for (int i = beginStat; i < endStat; i++) {
					if (!statCollidables->at(i).second.second.particle) {	//never check for collisions against particles
						PosSize aabb(
							statCollidables->at(i).second.first.position,
							statCollidables->at(i).second.second.size);
						qtreesStatic->at(physicsData->id).insert({ statCollidables->at(i).first, aabb });
					}
				}
			}

			// re sync with others after inserting (building trees)
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

			collisionInfos->reserve(dynCollidables->size() / 10.f);	//try to avoid reallocations

			std::vector<uint32_t> nearCollidables;	//reuse heap memory for all dyn collidable collisions
			nearCollidables.reserve(10);
			for (int i = beginDyn; i < endDyn; i++) {

				auto& coll = dynCollidables->at(i);
				auto& baseColl = world.getComp<Base>(coll.first);
				auto& colliderColl = world.getComp<Collider>(coll.first);
				(*collisionResponses)[coll.first].posChange = vec2(0, 0);
				nearCollidables.clear();

				vec2 collVel = (world.hasComp<Movement>(coll.first) ? world.getComp<Movement>(coll.first).velocity : vec2(0, 0));

				CollidableAdapter collAdapter = CollidableAdapter(world.getComp<Base>(coll.first).position, 
					world.getComp<Base>(coll.first).rotation, 
					collVel,
					world.getComp<Collider>(coll.first).size, 
					world.getComp<Collider>(coll.first).form, 
					world.getComp<Collider>(coll.first).dynamic);

				// querry dynamic entities
				for (int i = 0; i < physicsThreadCount; i++) {
					qtreesDynamic->at(i).querry(nearCollidables, baseColl.position, boundsSize(colliderColl.form, colliderColl.size, baseColl.rotation));
				}

				// querry static entities
				for (int i = 0; i < physicsThreadCount; i++) {
					qtreesStatic->at(i).querry(nearCollidables, baseColl.position, boundsSize(colliderColl.form, colliderColl.size, baseColl.rotation));
				}

				//check for collisions and save the changes in velocity and position these cause
				for (auto& otherID : nearCollidables) {
					//do not check against self 
					if (coll.first != otherID) {
						// check if one is a slave
						bool areCollidersRelated{ false };
						if (world.hasComp<Slave>(coll.first)) {
							if (world.getComp<Slave>(coll.first).ownerID == otherID) areCollidersRelated = true;
						}
						else if (world.hasComp<Slave>(otherID)){
							if (world.getComp<Slave>(otherID).ownerID == coll.first) areCollidersRelated = true;
						}
						//do not check for collision when the colliders are related (slave/owner)
						if (!areCollidersRelated) {
							vec2 velOther = (world.hasComp<Movement>(otherID) ? world.getComp<Movement>(otherID).velocity : vec2(0,0));
							CollidableAdapter otherAdapter = CollidableAdapter(world.getComp<Base>(otherID).position, world.getComp<Base>(otherID).rotation, velOther, world.getComp<Collider>(otherID).size, world.getComp<Collider>(otherID).form, world.getComp<Collider>(otherID).dynamic);
							auto newTestResult = checkForCollision(&collAdapter, &otherAdapter, world.hasComp<SolidBody>(coll.first) && world.hasComp<SolidBody>(otherID));

							if (newTestResult.collided) {
								collisionInfos->push_back(CollisionInfo(coll.first, otherID, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionPos));
								//take average of pushouts with weights
								float weightOld = norm((*collisionResponses)[coll.first].posChange);
								float weightNew = norm(newTestResult.posChange);
								float normalizer = weightOld + weightNew;
								if (normalizer > Physics::nullDelta) {
									(*collisionResponses)[coll.first].posChange = ((*collisionResponses)[coll.first].posChange * weightOld / normalizer + newTestResult.posChange * weightNew / normalizer);
								}
							}
						}
					}
				}
			}
		}
	}
}

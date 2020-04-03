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
				for (ent_id_t i = beginDyn; i < endDyn; i++) {
					if (!world.getComp<Collider>(dynCollidables->at(i)).particle) {	//never check for collisions against particles
						PosSize aabb(
							world.getComp<Base>(dynCollidables->at(i)).position,
							world.getComp<Collider>(dynCollidables->at(i)).size);
						qtreesDynamic->at(physicsData->id).insert({ dynCollidables->at(i), aabb });
					}
				}
			}
			// rebuild stat qtree
			if (physicsPoolData->rebuildStatQuadTrees) {
				for (ent_id_t i = beginStat; i < endStat; i++) {
					if (!world.getComp<Collider>(statCollidables->at(i)).particle) {	//never check for collisions against particles
						PosSize aabb(
							world.getComp<Base>(statCollidables->at(i)).position,
							world.getComp<Collider>(statCollidables->at(i)).size);
						qtreesStatic->at(physicsData->id).insert({ statCollidables->at(i), aabb });
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

			collisionInfos->reserve(static_cast<size_t>(dynCollidables->size() / 10.f));	//try to avoid reallocations

			std::vector<uint32_t> nearCollidables;	//reuse heap memory for all dyn collidable collisions
			nearCollidables.reserve(10);
			for (ent_id_t i = beginDyn; i < endDyn; i++) {

				auto& collID = dynCollidables->at(i);
				auto& baseColl = world.getComp<Base>(collID);
				auto& colliderColl = world.getComp<Collider>(collID);
				(*collisionResponses)[collID].posChange = vec2(0, 0);
				nearCollidables.clear();

				vec2 collVel = (world.hasComp<Movement>(collID) ? world.getComp<Movement>(collID).velocity : vec2(0, 0));

				CollidableAdapter collAdapter = CollidableAdapter(world.getComp<Base>(collID).position, 
					world.getComp<Base>(collID).rotation, 
					collVel,
					world.getComp<Collider>(collID).size, 
					world.getComp<Collider>(collID).form, 
					world.getComp<Collider>(collID).dynamic);

				// querry dynamic entities
				for (unsigned i = 0; i < physicsThreadCount; i++) {
					qtreesDynamic->at(i).querry(nearCollidables, baseColl.position, boundsSize(colliderColl.form, colliderColl.size, baseColl.rotation));
				}

				// querry static entities
				for (unsigned i = 0; i < physicsThreadCount; i++) {
					qtreesStatic->at(i).querry(nearCollidables, baseColl.position, boundsSize(colliderColl.form, colliderColl.size, baseColl.rotation));
				}

				//check for collisions and save the changes in velocity and position these cause
				for (auto& otherID : nearCollidables) {
					//do not check against self 
					if (collID != otherID) {
						// check if one is a slave
						bool areCollidersRelated{ false };
						if (world.hasComp<Slave>(collID) && world.hasComp<Slave>(otherID)) {	//same owner no collision check
							if (world.getComp<Slave>(collID).ownerID == world.getComp<Slave>(otherID).ownerID) areCollidersRelated = true;
						}
						else if (world.hasComp<Slave>(collID)) {
							if (world.getComp<Slave>(collID).ownerID == otherID) areCollidersRelated = true;
						}
						else if (world.hasComp<Slave>(otherID)){
							if (world.getComp<Slave>(otherID).ownerID == collID) areCollidersRelated = true;
						}
						//do not check for collision when the colliders are related (slave/owner)
						if (!areCollidersRelated) {
							vec2 velOther = (world.hasComp<Movement>(otherID) ? world.getComp<Movement>(otherID).velocity : vec2(0,0));
							CollidableAdapter otherAdapter = CollidableAdapter(world.getComp<Base>(otherID).position, world.getComp<Base>(otherID).rotation, velOther, world.getComp<Collider>(otherID).size, world.getComp<Collider>(otherID).form, world.getComp<Collider>(otherID).dynamic);
							auto newTestResult = checkForCollision(&collAdapter, &otherAdapter, world.hasComp<SolidBody>(collID) && world.hasComp<SolidBody>(otherID));

							if (newTestResult.collided) {
								collisionInfos->push_back(CollisionInfo(collID, otherID, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionPos));
								//take average of pushouts with weights
								float weightOld = norm((*collisionResponses)[collID].posChange);
								float weightNew = norm(newTestResult.posChange);
								float normalizer = weightOld + weightNew;
								if (normalizer > Physics::nullDelta) {
									(*collisionResponses)[collID].posChange = ((*collisionResponses)[collID].posChange * weightOld / normalizer + newTestResult.posChange * weightNew / normalizer);
								}
							}
						}
					}
				}
			}
		}
	}
}

#include "PhysicsWorker.h"
#include <algorithm>
#include <random>



void PhysicsWorker::operator()()
{
	auto& world = *physicsPoolData->world;
	auto& beginSensor = physicsData->beginSensor;
	auto& endSensor = physicsData->endSensor;
	auto& sensorCollidables = physicsPoolData->sensorCollidables;
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

	std::vector<uint32_t> nearCollidables;	//reuse heap memory for all dyn collidable collisions
	nearCollidables.reserve(100);

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
							boundsSize(world.getComp<Collider>(dynCollidables->at(i)).form, world.getComp<Collider>(dynCollidables->at(i)).size, world.getComp<Base>(dynCollidables->at(i)).rotation));
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
							boundsSize(world.getComp<Collider>(statCollidables->at(i)).form, world.getComp<Collider>(statCollidables->at(i)).size, world.getComp<Base>(statCollidables->at(i)).rotation));
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
			 
			// physics objects collisions
			for (ent_id_t i = beginDyn; i < endDyn; i++) {
				auto& collID = dynCollidables->at(i);

				CollidableAdapter collAdapter = CollidableAdapter(world.getComp<Base>(collID).position,
					world.getComp<Base>(collID).rotation,
					world.getComp<Movement>(collID).velocity,
					world.getComp<Collider>(collID).size,
					world.getComp<Collider>(collID).form,
					true);
				
				auto& baseColl = world.getComp<Base>(collID);
				auto& colliderColl = world.getComp<Collider>(collID);
				PosSize posSize(baseColl.position, boundsSize(colliderColl.form, colliderColl.size, baseColl.rotation));

				/// dyn vs dyn
				nearCollidables.clear();
				// querry dynamic entities
				for (unsigned i = 0; i < physicsThreadCount; i++) {
					qtreesDynamic->at(i).querry(nearCollidables, posSize);
				}

				//check for collisions and save the changes in velocity and position these cause
				
				for (auto& otherID : nearCollidables) {
					//do not check against self 
					if (collID != otherID) {
						//do not check for collision when the colliders are related (slave/owner)
						if (!areEntsRelated(collID, otherID)) {
							vec2 velOther = world.getComp<Movement>(otherID).velocity;	// direct access, as other ents all are dyn
							CollidableAdapter otherAdapter = CollidableAdapter(world.getComp<Base>(otherID).position, world.getComp<Base>(otherID).rotation, velOther, world.getComp<Collider>(otherID).size, world.getComp<Collider>(otherID).form, true);
							auto newTestResult = checkForCollision(&collAdapter, &otherAdapter);

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
				/// dyn vs static
				nearCollidables.clear();
				// querry static entities
				for (unsigned i = 0; i < physicsThreadCount; i++) {
					qtreesStatic->at(i).querry(nearCollidables, posSize);
				}

				for (auto& otherID : nearCollidables) {
					//do not check against self 
					if (collID != otherID) {
						//do not check for collision when the colliders are related (slave/owner)
						if (!areEntsRelated(collID, otherID)) {
							vec2 velOther{ 0,0 };	// 0 as all others are static
							CollidableAdapter otherAdapter = CollidableAdapter(world.getComp<Base>(otherID).position, world.getComp<Base>(otherID).rotation, velOther, world.getComp<Collider>(otherID).size, world.getComp<Collider>(otherID).form, false);
							auto newTestResult = checkForCollision(&collAdapter, &otherAdapter);

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

			// sensor collisions
			for (ent_id_t i = beginSensor; i < endSensor; i++) {
				auto& collID = sensorCollidables->at(i);
				auto& baseColl = world.getComp<Base>(collID);
				auto& colliderColl = world.getComp<Collider>(collID);
				(*collisionResponses)[collID].posChange = vec2(0, 0);
				nearCollidables.clear();

				CollidableAdapter collAdapter = CollidableAdapter(world.getComp<Base>(collID).position,
					world.getComp<Base>(collID).rotation,
					vec2(0,0),
					world.getComp<Collider>(collID).size,
					world.getComp<Collider>(collID).form,
					true);

				// querry dynamic entities
				for (unsigned i = 0; i < physicsThreadCount; i++) {
					PosSize posSize(baseColl.position, boundsSize(colliderColl.form, colliderColl.size, baseColl.rotation));
					qtreesDynamic->at(i).querry(nearCollidables, posSize);
				}

				// querry static entities
				if (!world.hasComp<MoveField>(collID)) {	// physics sensors ignore all static objects
					for (unsigned i = 0; i < physicsThreadCount; i++) {
						PosSize posSize(baseColl.position, boundsSize(colliderColl.form, colliderColl.size, baseColl.rotation));
						qtreesStatic->at(i).querry(nearCollidables, posSize);
					}
				}

				//check for collisions and save the changes in velocity and position these cause
				for (auto& otherID : nearCollidables) {
					//do not check against self 
					if (collID != otherID) {
						//do not check for collision when the colliders are related (slave/owner)
						if (!areEntsRelated(collID, otherID)) {
							vec2 velOther = (world.hasComp<Movement>(otherID) ? world.getComp<Movement>(otherID).velocity : vec2(0, 0));
							CollidableAdapter otherAdapter = CollidableAdapter(world.getComp<Base>(otherID).position, world.getComp<Base>(otherID).rotation, velOther, world.getComp<Collider>(otherID).size, world.getComp<Collider>(otherID).form, true);
							auto newTestResult = checkForCollision(&collAdapter, &otherAdapter);

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


#include "PhysicsWorker.h"
#include <algorithm>
#include <random>

void PhysicsWorker::operator()()
{
	auto& world = physicsPoolData->world;
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
	auto& qtreeDynamic = physicsPoolData->qtreeDynamic;
	auto& qtreeStatic = physicsPoolData->qtreeStatic;
	auto& aabbCache = physicsPoolData->aabbCache;

	bool firstIteration = true;
	while (run){
		if (!firstIteration) {
			if (physicsData->id == 0) {
				if (physicsPoolData->rebuildDynQuadTrees) {
					for (auto& ent : dynCollidables) {
						if (!world.getComp<Collider>(ent).particle) {	// never check for collisions against particles
							qtreeDynamic.insert(ent);
						}
					}
				}
				// rebuild stat qtree
				if (physicsThreadCount == 1) {
					if (physicsPoolData->rebuildStatQuadTrees) {
						for (auto& ent : statCollidables) {
							if (!world.getComp<Collider>(ent).particle) {	// never check for collisions against particles
								qtreeStatic.insert(ent);
							}
						}
					}
					cacheAABBs(dynCollidables);
					cacheAABBs(statCollidables);
					cacheAABBs(sensorCollidables);
				}
			}
			if (physicsData->id == 1) {
				if (physicsPoolData->rebuildStatQuadTrees) {
					for (auto& ent : statCollidables) {
						if (!world.getComp<Collider>(ent).particle) {	// never check for collisions against particles
							qtreeStatic.insert(ent);
						}
					}
				}
				cacheAABBs(dynCollidables);
				cacheAABBs(statCollidables);
				cacheAABBs(sensorCollidables);
			}

			// re sync with others after building trees
			waitForOtherWorkers();

			collisionInfos->reserve(static_cast<size_t>(dynCollidables.size() / 10.f));	// try to avoid reallocations

			// physics objects collision tests
			for (uint32_t i = beginDyn; i < endDyn; i++) {
				auto& collID = dynCollidables.at(i);
				collisionFunction(collID, qtreeDynamic, true);
				collisionFunction(collID, qtreeStatic, false);
			}

			// sensor collision tests
			for (uint32_t i = beginSensor; i < endSensor; i++) {
				auto& collID = sensorCollidables.at(i);
				collisionFunction(collID, qtreeDynamic, true);
				if (world.hasntComps<LinearEffector, FrictionEffector>(collID)) {
					collisionFunction(collID, qtreeStatic, false);
				}
			}
		}
		else {
			firstIteration = false;
		}
		waitForUpdate();
	}
}


void PhysicsWorker::cacheAABBs(std::vector<ent_id_t>& colliders) {
	for (auto ent : colliders) {
		auto& base = physicsPoolData->world.getComp<Base>(ent);
		auto& collider = physicsPoolData->world.getComp<Collider>(ent);
		physicsPoolData->aabbCache.at(ent) = aabbBounds(collider.size, base.rotaVec);
	}
}

void PhysicsWorker::waitForUpdate() {
	std::unique_lock<std::mutex> switch_lock(syncData->mut);
	syncData->go.at(physicsData->id) = false;
	syncData->cond.notify_all();
	syncData->cond.wait(switch_lock, [&]() { return syncData->go.at(physicsData->id) == true; });	// wait for engine to give go sign
	if (syncData->run == false) run = false;
}

void PhysicsWorker::waitForOtherWorkers()
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

void PhysicsWorker::collisionFunction(ent_id_t collID, Quadtree2 const& quadtree, bool otherDynamic) {
	auto& world = physicsPoolData->world;
	auto& collisionResponses = physicsPoolData->collisionResponses;

	if (!world.getComp<Collider>(collID).sleeping) {

		CollidableAdapter collAdapter = CollidableAdapter(
			world.getComp<Base>(collID).position,
			world.getComp<Base>(collID).rotation,
			world.getComp<Collider>(collID).size,
			world.getComp<Collider>(collID).form,
			true,
			world.getComp<Base>(collID).rotaVec);

		auto& baseColl = world.getComp<Base>(collID);
		auto& colliderColl = world.getComp<Collider>(collID);
		PosSize posSize(baseColl.position, aabbBounds(colliderColl.size, baseColl.rotaVec));

		/// dyn vs dyn
		nearCollidablesBuffer.clear();
		// querry dynamic entities
		quadtree.querry(nearCollidablesBuffer, posSize);

		//check for collisions and save the changes in velocity and position these cause

		for (auto& otherID : nearCollidablesBuffer) {
			//do not check against self 
			if (collID != otherID) {
				//do not check for collision when the colliders are related (slave/owner)
				if (!world.areEntsRelated(collID, otherID)) {
					CollidableAdapter otherAdapter = CollidableAdapter(
						world.getComp<Base>(otherID).position,
						world.getComp<Base>(otherID).rotation,
						world.getComp<Collider>(otherID).size,
						world.getComp<Collider>(otherID).form,
						otherDynamic,
						world.getComp<Base>(otherID).rotaVec);
					auto newTestResult = collisionTestCachedAABB(collAdapter, otherAdapter, physicsPoolData->aabbCache.at(collID), physicsPoolData->aabbCache.at(otherID));

					if (newTestResult.collided) {
						physicsData->collisionInfos->push_back(CollisionInfo(collID, otherID, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionPos));
						if (world.hasComp<Movement>(collID)) {
							auto& movementColl = world.getComp<Movement>(collID);
							Vec2 velocityOther;
							if (otherDynamic) {
								auto& movementOther = world.getComp<Movement>(otherID);
								velocityOther = movementOther.velocity;
							}
							else {
								velocityOther = Vec2(0, 0);
							}

							Vec2 newPosChange = calcPosChange(
								collAdapter.getSurfaceArea(), movementColl.velocity,
								otherAdapter.getSurfaceArea(), velocityOther,
								newTestResult.clippingDist, newTestResult.collisionNormal, otherDynamic);

							Vec2 oldPosChange = collisionResponses.at(collID).posChange;

							float weightOld = norm(oldPosChange);
							float weightNew = norm(newPosChange);
							float normalizer = weightOld + weightNew;
							if (normalizer > Physics::nullDelta) {
								collisionResponses.at(collID).posChange = (oldPosChange * weightOld / normalizer + newPosChange * weightNew / normalizer);
							}
						}
					}
				}
			}
		}
	}
}
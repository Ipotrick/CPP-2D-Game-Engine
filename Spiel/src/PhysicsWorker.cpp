#include "PhysicsWorker.h"
#include <algorithm>
#include <random>

void PhysicsWorker::operator()()
{
	auto& world = poolData->world;
	auto& beginSensor = physicsData->beginSensor;
	auto& endSensor = physicsData->endSensor;
	auto& sensorCollidables = poolData->sensorCollidables;
	auto& beginDyn = physicsData->beginDyn;
	auto& endDyn = physicsData->endDyn;
	auto& dynCollidables = poolData->dynCollidables;
	auto& beginStat = physicsData->beginStat;
	auto& endStat = physicsData->endStat;
	auto& statCollidables = poolData->statCollidables;
	auto& collisionResponses = poolData->collisionResponses;
	auto& collisionInfos = physicsData->collisionInfos;
	auto& qtreeDynamic = poolData->qtreeDynamic;
	auto& qtreeStatic = poolData->qtreeStatic;
	auto& aabbCache = poolData->aabbCache;

	bool firstIteration = true;
	while (run){
		if (!firstIteration) {
			if (physicsData->id == 0) {
				if (poolData->rebuildDynQuadTrees) {
					for (auto& ent : dynCollidables) {
						if (!world.getComp<Collider>(ent).particle) {	// never check for collisions against particles
							qtreeDynamic.insert(ent);
						}
					}
				}
				// rebuild stat qtree
				if (physicsThreadCount == 1) {
					if (poolData->rebuildStatQuadTrees) {
						for (auto& ent : statCollidables) {
							if (!world.getComp<Collider>(ent).particle) {	// never check for collisions against particles
								qtreeStatic.insert(ent);
							}
						}
						updateStaticGrid();
					}
					cacheAABBs(dynCollidables);
					cacheAABBs(statCollidables);
					cacheAABBs(sensorCollidables);
				}
			}
			if (physicsData->id == 1) {
				if (poolData->rebuildStatQuadTrees) {
					for (auto& ent : statCollidables) {
						if (!world.getComp<Collider>(ent).particle) {	// never check for collisions against particles
							qtreeStatic.insert(ent);
						}
					}
					updateStaticGrid();
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


void PhysicsWorker::cacheAABBs(std::vector<entity_handle>& colliders) {
	for (auto ent : colliders) {
		auto& base = poolData->world.getComp<Base>(ent);
		auto& collider = poolData->world.getComp<Collider>(ent);
		poolData->aabbCache.at(ent) = aabbBounds(collider.size, base.rotaVec);
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

void PhysicsWorker::collisionFunction(entity_handle collID, Quadtree2 const& quadtree, bool otherDynamic) {
	auto& world = poolData->world;
	auto& collisionResponses = poolData->collisionResponses;

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
				CollidableAdapter otherAdapter = CollidableAdapter(
					world.getComp<Base>(otherID).position,
					world.getComp<Base>(otherID).rotation,
					world.getComp<Collider>(otherID).size,
					world.getComp<Collider>(otherID).form,
					otherDynamic,
					world.getComp<Base>(otherID).rotaVec);
				auto newTestResult = collisionTestCachedAABB(collAdapter, otherAdapter, poolData->aabbCache.at(collID), poolData->aabbCache.at(otherID));

				if (newTestResult.collided) {
					if (!world.areEntsRelated(collID, otherID)) {
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

void PhysicsWorker::updateStaticGrid()
{
	if (poolData->world.didStaticsChange()) {
		Vec2 staticGridMinPos = Vec2(0, 0);
		Vec2 staticGridMaxPos = Vec2(0, 0);
		auto treeMin = poolData->qtreeStatic.getPosition() - poolData->qtreeStatic.getSize() * 0.5f;
		auto treeMax = poolData->qtreeStatic.getPosition() + poolData->qtreeStatic.getSize() * 0.5f;
		if (treeMin.x < staticGridMinPos.x) staticGridMinPos.x = treeMin.x;
		if (treeMin.y < staticGridMinPos.y) staticGridMinPos.y = treeMin.y;
		if (treeMax.x > staticGridMaxPos.x) staticGridMaxPos.x = treeMax.x;
		if (treeMax.y > staticGridMaxPos.y) staticGridMaxPos.y = treeMax.y;
		poolData->staticCollisionGrid.minPos = staticGridMinPos;
		int xSize = static_cast<int>(ceilf((staticGridMaxPos.x - staticGridMinPos.x)) / poolData->staticCollisionGrid.cellSize.x);
		int ySize = static_cast<int>(ceilf((staticGridMaxPos.y - staticGridMinPos.y)) / poolData->staticCollisionGrid.cellSize.y);
		poolData->staticCollisionGrid.clear();
		poolData->staticCollisionGrid.resize(xSize, ySize);

		std::vector<uint32_t> nearCollidables;
		nearCollidables.reserve(20);
		for (int x = 0; x < poolData->staticCollisionGrid.getSizeX(); x++) {
			for (int y = 0; y < poolData->staticCollisionGrid.getSizeY(); y++) {
				Vec2 pos = poolData->staticCollisionGrid.minPos + Vec2(x, y) * poolData->staticCollisionGrid.cellSize;
				Vec2 size = poolData->staticCollisionGrid.cellSize;
				CollidableAdapter collAdapter = CollidableAdapter(pos, 0, size, Form::RECTANGLE, true, RotaVec2());

				PosSize posSize(pos, size);
				poolData->qtreeStatic.querry(nearCollidables, posSize);

				for (auto& otherID : nearCollidables) {
					CollidableAdapter otherAdapter = CollidableAdapter(poolData->world.getComp<Base>(otherID).position, poolData->world.getComp<Base>(otherID).rotation, poolData->world.getComp<Collider>(otherID).size, poolData->world.getComp<Collider>(otherID).form, false, poolData->world.getComp<Base>(otherID).rotaVec);
					auto result = collisionTest(collAdapter, otherAdapter);
					if (result.collided) {
						poolData->staticCollisionGrid.set(x, y, true);
						break;
					}
				}
				nearCollidables.clear();
			}
		}
	}
#define DEBUG_STATIC_GRID
#ifdef DEBUG_STATIC_GRID
	Drawable d = Drawable(0, poolData->staticCollisionGrid.minPos, 0.1f, poolData->staticCollisionGrid.cellSize, Vec4(1, 1, 0, 1), Form::RECTANGLE, 0.0f);
	for (int i = 0; i < poolData->staticCollisionGrid.getSizeX(); i++) {
		for (int j = 0; j < poolData->staticCollisionGrid.getSizeY(); j++) {
			d.position = poolData->staticCollisionGrid.minPos + Vec2(i, 0) * poolData->staticCollisionGrid.cellSize.x + Vec2(0, j) * poolData->staticCollisionGrid.cellSize.y;
			if (poolData->staticCollisionGrid.at(i, j)) {
				poolData->debugDrawables.push_back(d);
			}
		}
	}
#endif
}
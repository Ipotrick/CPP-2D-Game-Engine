#pragma once
#include "CollisionSystem.h"

#include "PhysicsSystem.h"
#include "CacheAABBJob.hpp"
#include "BuildQtreeJob.hpp"
#include "CollisionCheckJob.h"

CollisionSystem::CollisionSystem(World& world, uint32_t threadCount, PerfLogger& perfLog, float statCollGridRes, uint32_t qtreeCapacity) :
	CoreSystem(world),
	threadCount{ threadCount },
	perfLog{ perfLog },
	qtreeCapacity{ qtreeCapacity },
	jobManager(6)
{
	poolWorkerData = std::make_shared<CollisionPoolData>(CollisionPoolData(world, qtreeCapacity));
	poolWorkerData->staticCollisionGrid = GridPhysics<bool>(statCollGridRes);

	perWorkerData = std::vector<std::shared_ptr<CollisionWorkerData>>(threadCount);
	for (int id = 0; id < threadCount; id++) {
		perWorkerData[id] = std::make_shared<CollisionWorkerData>();
		perWorkerData[id]->id = id;
	}
	for (int i = 0; i < threadCount * 2; i++) {
		neighborBuffer.push_back(std::vector<entity_index_type>());
	}

	syncWorkerData = std::make_shared<CollisionSyncData>();
	syncWorkerData->go = std::vector<bool>(threadCount, false);
	for (unsigned i = 0; i < threadCount; i++) {
		workerThreads.push_back(std::thread(CollisionWorker(perWorkerData.at(i), poolWorkerData, syncWorkerData, threadCount)));
		workerThreads.at(i).detach();
	}

	collisionInfosSplit.resize(threadCount);
}

void CollisionSystem::execute(World& world, float deltaTime)
{
	prepare();
	collisionDetection();
	debugDrawables.insert(debugDrawables.end(), poolWorkerData->debugDrawables.begin(), poolWorkerData->debugDrawables.end());
}

std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> CollisionSystem::getCollisions(entity_index_type entity)
{
	auto begin = collisionInfoBegins.find(entity);
	auto end = collisionInfoEnds.find(entity);
	if (begin != collisionInfoBegins.end() && end != collisionInfoEnds.end()) {	// is there even collisionInfo for the id?
		return std::make_tuple(begin->second, end->second);
	}
	else {
		return std::make_tuple(collisionInfos.end(), collisionInfos.end());
	}
}

std::vector<CollisionInfo>& CollisionSystem::getAllCollisions()
{
	return this->collisionInfos;
}

GridPhysics<bool> CollisionSystem::getStaticGrid()
{
	return poolWorkerData->staticCollisionGrid;
}

void CollisionSystem::end()
{
	syncWorkerData->run = false;
	std::unique_lock switch_lock(syncWorkerData->mut);
	for (unsigned i = 0; i < threadCount; i++) {
		syncWorkerData->go.at(i) = true;
	}
	jobManager.end();
}

void CollisionSystem::prepare()
{
	Timer t1(perfLog.getInputRef("physicsprepare"));

	poolWorkerData->rebuildDynQuadTrees = true; // allways rebuild dynamic quadtree
	poolWorkerData->rebuildStatQuadTrees = world.didStaticsChange(); // only rebuild static quadtree if static Entities changed

	// allocate memory for collider groups
	cleanBuffers();

	Vec2 sensorMaxPos{ 0,0 }, sensorMinPos{ 0,0 };
	Vec2 dynMaxPos{ 0,0 }, dynMinPos{ 0,0 };
	Vec2 statMaxPos{ 0,0 }, statMinPos{ 0,0 };
	for (auto colliderID : world.viewIDX<Collider>()) {
		auto& collider = world.getComp<Collider>(colliderID);
		auto& baseCollider = world.getComp<Base>(colliderID);

		if (world.hasComp<PhysicsBody>(colliderID)) { // if a collider has a solidBody, it is a physics object
			if (world.hasComp<Movement>(colliderID)) {	// is it dynamic or static?
				poolWorkerData->dynCollidables.push_back(colliderID);
				dynMaxPos = max(dynMaxPos, baseCollider.position);
				dynMinPos = min(dynMinPos, baseCollider.position);
			}
			else {	// entity must be static
				poolWorkerData->statCollidables.push_back(colliderID);
				statMaxPos = max(statMaxPos, baseCollider.position);
				statMinPos = min(statMinPos, baseCollider.position);
			}
		}
		else { // if a collider has NO PhysicsBody, it is a sensor
			poolWorkerData->sensorCollidables.push_back(colliderID);
			sensorMaxPos = max(sensorMaxPos, baseCollider.position);
			sensorMinPos = min(sensorMinPos, baseCollider.position);
		}
	}

	// split the entities between threads 
	auto calcSplit = [&](float splitSize) -> std::vector<std::array<int, 2>> {
		std::vector<std::array<int, 2>> res(threadCount);
		for (unsigned i = 0; i < threadCount; i++) {
			res[i][0] = (int)(floorf(i * splitSize));
			res[i][1] = (int)(floorf((i + 1) * splitSize));
		}
		return res;
	};

	float splitStepSensor = (float)poolWorkerData->sensorCollidables.size() / (float)(threadCount);
	std::vector<std::array<int, 2>> rangesSensor = calcSplit(splitStepSensor);
	float splitStepDyn = (float)poolWorkerData->dynCollidables.size() / (float)(threadCount);
	std::vector<std::array<int, 2>> rangesDyn = calcSplit(splitStepDyn);
	float splitStepStat = (float)poolWorkerData->statCollidables.size() / (float)(threadCount);
	std::vector<std::array<int, 2>> rangesStat = calcSplit(splitStepStat);


	// give physics workers their info
	// write worker pool data
	if (poolWorkerData->rebuildDynQuadTrees) {
		poolWorkerData->qtreeDynamic.resetPerMinMax(dynMinPos, dynMaxPos);
		poolWorkerData->qtreeDynamic.removeEmptyLeafes();
	}
	if (poolWorkerData->rebuildStatQuadTrees) {
		poolWorkerData->qtreeStatic.resetPerMinMax(statMinPos, statMaxPos);
		poolWorkerData->qtreeStatic.removeEmptyLeafes();
	}
	// write individual worker data
	for (unsigned i = 0; i < threadCount; i++) {
		auto& pData = perWorkerData[i];
		pData->beginSensor = rangesSensor[i][0];
		pData->endSensor = rangesSensor[i][1];
		pData->beginDyn = rangesDyn[i][0];
		pData->endDyn = rangesDyn[i][1];
		pData->beginStat = rangesStat[i][0];
		pData->endStat = rangesStat[i][1];
		pData->collisionInfos = &collisionInfosSplit[i];
	}
	t1.stop();

	// start jobs to prepare buffers

	int jobTagStaticRebuild = -1;
	BuildQtreeJob staticQtreeBuildJob = BuildQtreeJob(world, poolWorkerData->statCollidables, poolWorkerData->qtreeStatic);
	if (poolWorkerData->rebuildStatQuadTrees) {
		jobTagStaticRebuild = jobManager.addJob(&staticQtreeBuildJob);
	}

	int jobTagQtreeDyn = -1;
	BuildQtreeJob dynqTreeJob = BuildQtreeJob(world, poolWorkerData->dynCollidables, poolWorkerData->qtreeDynamic);
	if (poolWorkerData->rebuildDynQuadTrees) {
		jobTagQtreeDyn = jobManager.addJob(&dynqTreeJob);
	}
	
	CacheAABBJob aabbJob1 = CacheAABBJob(poolWorkerData->dynCollidables, world, poolWorkerData->aabbCache);
	auto jobTagAABBDyn = jobManager.addJob(&aabbJob1);

	CacheAABBJob aabbJob2 = CacheAABBJob(poolWorkerData->statCollidables, world, poolWorkerData->aabbCache);
	auto jobTagAABBStat = jobManager.addJob(&aabbJob2);

	CacheAABBJob aabbJob3 = CacheAABBJob(poolWorkerData->sensorCollidables, world, poolWorkerData->aabbCache);
	auto jobTagAABBSensor = jobManager.addJob(&aabbJob3);

	jobManager.waitFor(jobTagAABBDyn);
	jobManager.waitFor(jobTagAABBStat);
	jobManager.waitFor(jobTagAABBSensor);
	if (poolWorkerData->rebuildStatQuadTrees)
		jobManager.waitFor(jobTagStaticRebuild);
	if (poolWorkerData->rebuildDynQuadTrees)
		jobManager.waitFor(jobTagQtreeDyn);

	///------------------------------------
	///TODO MOVE THIS TO OTHER FUNCTION::::
	///------------------------------------

	DynCollisionCheckJob collChecker0 = DynCollisionCheckJob(poolWorkerData, poolWorkerData->dynCollidables, *(perWorkerData[0]->collisionInfos), rangesDyn[0][0], rangesDyn[0][1], neighborBuffer[0]);
	DynCollisionCheckJob collChecker1 = DynCollisionCheckJob(poolWorkerData, poolWorkerData->dynCollidables, *(perWorkerData[1]->collisionInfos), rangesDyn[1][0], rangesDyn[1][1], neighborBuffer[1]);
	DynCollisionCheckJob collChecker2 = DynCollisionCheckJob(poolWorkerData, poolWorkerData->dynCollidables, *(perWorkerData[2]->collisionInfos), rangesDyn[2][0], rangesDyn[2][1], neighborBuffer[2]);
	DynCollisionCheckJob collChecker3 = DynCollisionCheckJob(poolWorkerData, poolWorkerData->dynCollidables, *(perWorkerData[3]->collisionInfos), rangesDyn[3][0], rangesDyn[3][1], neighborBuffer[3]);
	DynCollisionCheckJob collChecker4 = DynCollisionCheckJob(poolWorkerData, poolWorkerData->dynCollidables, *(perWorkerData[4]->collisionInfos), rangesDyn[4][0], rangesDyn[4][1], neighborBuffer[4]);
	uint32_t tag0 = jobManager.addJob(&collChecker0);
	uint32_t tag1 = jobManager.addJob(&collChecker1);
	uint32_t tag2 = jobManager.addJob(&collChecker2);
	uint32_t tag3 = jobManager.addJob(&collChecker3);
	uint32_t tag4 = jobManager.addJob(&collChecker4);
	SensorCollisionCheckJob sensorChecker0 = SensorCollisionCheckJob(poolWorkerData, poolWorkerData->sensorCollidables, *(perWorkerData[0]->collisionInfos), rangesSensor[0][0], rangesSensor[0][1], neighborBuffer[5 + 0]);
	SensorCollisionCheckJob sensorChecker1 = SensorCollisionCheckJob(poolWorkerData, poolWorkerData->sensorCollidables, *(perWorkerData[1]->collisionInfos), rangesSensor[1][0], rangesSensor[1][1], neighborBuffer[5 + 1]);
	SensorCollisionCheckJob sensorChecker2 = SensorCollisionCheckJob(poolWorkerData, poolWorkerData->sensorCollidables, *(perWorkerData[2]->collisionInfos), rangesSensor[2][0], rangesSensor[2][1], neighborBuffer[5 + 2]);
	SensorCollisionCheckJob sensorChecker3 = SensorCollisionCheckJob(poolWorkerData, poolWorkerData->sensorCollidables, *(perWorkerData[3]->collisionInfos), rangesSensor[3][0], rangesSensor[3][1], neighborBuffer[5 + 3]);
	SensorCollisionCheckJob sensorChecker4 = SensorCollisionCheckJob(poolWorkerData, poolWorkerData->sensorCollidables, *(perWorkerData[4]->collisionInfos), rangesSensor[4][0], rangesSensor[4][1], neighborBuffer[5 + 3]);
	uint32_t tag5 = jobManager.addJob(&sensorChecker0);
	uint32_t tag6 = jobManager.addJob(&sensorChecker1);
	uint32_t tag7 = jobManager.addJob(&sensorChecker2);
	uint32_t tag8 = jobManager.addJob(&sensorChecker3);
	uint32_t tag9 = jobManager.addJob(&sensorChecker4);


	jobManager.waitFor(tag0);
	jobManager.waitFor(tag1);
	jobManager.waitFor(tag2);
	jobManager.waitFor(tag3);
	jobManager.waitFor(tag4);
	jobManager.waitFor(tag5);
	jobManager.waitFor(tag6);
	jobManager.waitFor(tag7);
	jobManager.waitFor(tag8);
	jobManager.waitFor(tag9);
}

void CollisionSystem::cleanBuffers()
{
	auto cleanAndShrink = [this] (auto& vector) {
		if (vector.capacity() >= vector.size() * 50) {
			vector.shrink_to_fit();
		}
		vector.clear();
	};

	cleanAndShrink(debugDrawables);
	cleanAndShrink(poolWorkerData->debugDrawables);
	cleanAndShrink(poolWorkerData->sensorCollidables);
	cleanAndShrink(poolWorkerData->dynCollidables);
	cleanAndShrink(poolWorkerData->statCollidables);
	cleanAndShrink(poolWorkerData->collisionResponses);
	if (poolWorkerData->collisionResponses.size() != world.memorySize()) poolWorkerData->collisionResponses.resize(world.memorySize());
	cleanAndShrink(poolWorkerData->aabbCache);
	if (poolWorkerData->aabbCache.size() != world.memorySize()) poolWorkerData->aabbCache.resize(world.memorySize());
	for (auto& split : collisionInfosSplit) cleanAndShrink(split);
	cleanAndShrink(collisionInfos);
	collisionInfoBegins.clear();
	collisionInfoEnds.clear();
}

void CollisionSystem::collisionDetection()
{
	Timer t2(perfLog.getInputRef("physicscollide"));

	//{	// start collision check threads
	//	std::unique_lock switch_lock(syncWorkerData->mut);
	//	for (unsigned i = 0; i < threadCount; i++) {
	//		syncWorkerData->go.at(i) = true;
	//	}
	//	syncWorkerData->cond.notify_all();
	//	// wait for physics threads to finish
	//	syncWorkerData->cond.wait(switch_lock, [&]() {
	//		/* wenn alle false sind wird true returned */
	//		for (unsigned i = 0; i < threadCount; i++) {
	//			if (syncWorkerData->go.at(i) == true) {
	//				return false;
	//			}
	//		}
	//		return true;
	//		}
	//	);
	//}
	// reset quadtree rebuild flags
	poolWorkerData->rebuildDynQuadTrees = false;
	poolWorkerData->rebuildStatQuadTrees = false;

	// store all collisioninfos in one vectorcapacity
	for (auto collisionInfosplit : collisionInfosSplit) {
		collisionInfos.insert(collisionInfos.end(), collisionInfosplit.begin(), collisionInfosplit.end());
	}

	// build hashtables for first and last iterator element of collisioninfo
	uint32_t lastIDA{};
	for (auto iter = collisionInfos.begin(); iter != collisionInfos.end(); ++iter) {
		if (iter == collisionInfos.begin()) {	//initialize values from first element
			lastIDA = iter->idA;
			collisionInfoBegins.insert({ iter->idA, iter });
		}
		if (lastIDA != iter->idA) {	//new idA found
			collisionInfoEnds.insert({ lastIDA, iter });
			collisionInfoBegins.insert({ iter->idA, iter });
			lastIDA = iter->idA;	//set lastId to new id
		}
	}
	collisionInfoEnds.insert({ lastIDA, collisionInfos.end() });

	t2.stop();
}
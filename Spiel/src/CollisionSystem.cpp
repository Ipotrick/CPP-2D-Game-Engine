#pragma once
#include "CollisionSystem.h"

#include "PhysicsSystem.h"
#include "CacheAABBJob.hpp"
#include "BuildQtreeJob.hpp"

CollisionSystem::CollisionSystem(World& world, JobManager& jobManager, PerfLogger& perfLog, float statCollGridRes, uint32_t qtreeCapacity) :
	CoreSystem(world),
	jobManager{ jobManager },
	perfLog{ perfLog },
	qtreeCapacity{ qtreeCapacity }
{
	poolWorkerData = std::make_shared<CollisionPoolData>(world, qtreeCapacity, jobManager.workerCount());
}

void CollisionSystem::execute(World& world, float deltaTime)
{
	poolWorkerData->world = world;
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
}

void CollisionSystem::prepare()
{
	Timer t1(perfLog.getInputRef("physicsprepare"));

	poolWorkerData->rebuildDynQuadTrees = true; // allways rebuild dynamic quadtree
	poolWorkerData->rebuildStatQuadTrees = world.didStaticsChange(); // only rebuild static quadtree if static Entities changed

	// allocate memory for collider groups
	cleanBuffers();

	// split collidables
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

	// clean quadtrees
	if (poolWorkerData->rebuildDynQuadTrees) {
		poolWorkerData->qtreeDynamic.resetPerMinMax(dynMinPos, dynMaxPos);
		poolWorkerData->qtreeDynamic.removeEmptyLeafes();
	}
	if (poolWorkerData->rebuildStatQuadTrees) {
		poolWorkerData->qtreeStatic.resetPerMinMax(statMinPos, statMaxPos);
		poolWorkerData->qtreeStatic.removeEmptyLeafes();
	}

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
	for (auto& split : poolWorkerData->collisionInfoBuffers) cleanAndShrink(split);
	cleanAndShrink(collisionInfos);
	collisionInfoBegins.clear();
	collisionInfoEnds.clear();

	cleanAndShrink(dynCheckJobs);
	cleanAndShrink(dynTags);
	cleanAndShrink(sensorCheckJobs);
	cleanAndShrink(sensorTags);
}

void CollisionSystem::collisionDetection()
{
	Timer t2(perfLog.getInputRef("physicscollide"));

	int entityCount = 0;
	for (int i = 0; i < poolWorkerData->dynCollidables.size(); i++) {
		if (entityCount == jobMaxEntityCount) {
			dynCheckJobs.push_back(DynCollisionCheckJob(poolWorkerData, poolWorkerData->dynCollidables, i - jobMaxEntityCount,i));
			entityCount = 0;
		}
		entityCount++;
	}
	dynCheckJobs.push_back(DynCollisionCheckJob(poolWorkerData, poolWorkerData->dynCollidables, poolWorkerData->dynCollidables.size()-entityCount, poolWorkerData->dynCollidables.size()));

	for (auto& job : dynCheckJobs) {
		int tag = jobManager.addJob(&job);
		dynTags.push_back(tag);
	}

	entityCount = 0;
	for (int i = 0; i < poolWorkerData->sensorCollidables.size(); i++) {
		if (entityCount == jobMaxEntityCount) {
			sensorCheckJobs.push_back(SensorCollisionCheckJob(poolWorkerData, poolWorkerData->sensorCollidables, i - jobMaxEntityCount, i));
			entityCount = 0;
		}
		entityCount++;
	}
	sensorCheckJobs.push_back(SensorCollisionCheckJob(poolWorkerData, poolWorkerData->sensorCollidables, poolWorkerData->sensorCollidables.size() - entityCount, poolWorkerData->sensorCollidables.size()));

	for (auto& job : sensorCheckJobs) {
		sensorTags.push_back(jobManager.addJob(&job));
	}

	for (auto& tag : dynTags) {
		jobManager.waitFor(tag);
	}

	for (auto& tag : sensorTags) {
		jobManager.waitFor(tag);
	}

	// reset quadtree rebuild flags
	poolWorkerData->rebuildDynQuadTrees = false;
	poolWorkerData->rebuildStatQuadTrees = false;

	// store all collisioninfos in one vectorcapacity
	for (auto collisionInfosplit : poolWorkerData->collisionInfoBuffers) {
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
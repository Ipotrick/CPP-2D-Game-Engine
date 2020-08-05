#pragma once
#include "CollisionSystem.hpp"

#include "PhysicsSystem.hpp"
#include "CacheAABBJob.hpp"
#include "BuildQtreeJob.hpp"

CollisionSystem::CollisionSystem(World& world, JobManager& jobManager, PerfLogger& perfLog, float statCollGridRes, uint32_t qtreeCapacity) :
	jobManager{ jobManager },
	perfLog{ perfLog },
	qtreeCapacity{ qtreeCapacity }
{
	poolWorkerData = std::make_shared<CollisionPoolData>(world, qtreeCapacity, jobManager.neededBufferNum());
}

void CollisionSystem::execute(World& world, float deltaTime)
{
	prepare(world);
	collisionDetection(world);
	debugDrawables.insert(debugDrawables.end(), poolWorkerData->debugDrawables.begin(), poolWorkerData->debugDrawables.end());
}

std::tuple<std::vector<IndexCollisionInfo>::iterator, std::vector<IndexCollisionInfo>::iterator> CollisionSystem::getCollisions(Entity entity)
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

std::vector<IndexCollisionInfo>& CollisionSystem::getAllCollisions()
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

void CollisionSystem::prepare(World& world)
{
	Timer t1(perfLog.getInputRef("collisionprepare"));
	poolWorkerData->world = world;
	poolWorkerData->qtreeDynamic.world = world;
	poolWorkerData->qtreeStatic.world = world;

	poolWorkerData->rebuildDynQuadTrees = true; // allways rebuild dynamic quadtree
	poolWorkerData->rebuildStatQuadTrees = world.didStaticsChange(); // only rebuild static quadtree if static Entities changed

	// allocate memory for collider groups and or clean them
	cleanBuffers(world);

	// split collidables
	Vec2 sensorMaxPos{ 0,0 }, sensorMinPos{ 0,0 };
	Vec2 dynMaxPos{ 0,0 }, dynMinPos{ 0,0 };
	Vec2 statMaxPos{ 0,0 }, statMinPos{ 0,0 };
	for (auto colliderID : world.entity_view<Collider>()) {
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
	t1.stop();
	Timer t2(perfLog.getInputRef("collisionbroad"));
	// clean quadtrees
	if (poolWorkerData->rebuildDynQuadTrees) {
		poolWorkerData->qtreeDynamic.resetPerMinMax(dynMinPos, dynMaxPos);
		poolWorkerData->qtreeDynamic.removeEmptyLeafes();
	}
	if (poolWorkerData->rebuildStatQuadTrees) {
		poolWorkerData->qtreeStatic.resetPerMinMax(statMinPos, statMaxPos);
		poolWorkerData->qtreeStatic.removeEmptyLeafes();
	}
	poolWorkerData->qtreeParticle.resetPerMinMax(statMinPos, statMaxPos);


	CacheAABBJob aabbJob1 = CacheAABBJob(poolWorkerData->dynCollidables, world, poolWorkerData->aabbCache);
	auto jobTagAABBDyn = jobManager.addJob(&aabbJob1);

	CacheAABBJob aabbJob2 = CacheAABBJob(poolWorkerData->statCollidables, world, poolWorkerData->aabbCache);
	auto jobTagAABBStat = jobManager.addJob(&aabbJob2);

	CacheAABBJob aabbJob3 = CacheAABBJob(poolWorkerData->sensorCollidables, world, poolWorkerData->aabbCache);
	auto jobTagAABBSensor = jobManager.addJob(&aabbJob3);

	jobManager.waitFor(jobTagAABBDyn);
	jobManager.waitFor(jobTagAABBStat);
	jobManager.waitFor(jobTagAABBSensor);

	// start jobs to prepare buffers
	int jobTagStaticRebuild = -1;
	BuildQtreeJob staticQtreeBuildJob = BuildQtreeJob(world, poolWorkerData->statCollidables, poolWorkerData->qtreeStatic, poolWorkerData->aabbCache, false);
	if (poolWorkerData->rebuildStatQuadTrees) {
		jobTagStaticRebuild = jobManager.addJob(&staticQtreeBuildJob);
	}

	int jobTagQtreeDyn = -1;
	BuildQtreeJob dynqTreeJob = BuildQtreeJob(world, poolWorkerData->dynCollidables, poolWorkerData->qtreeDynamic, poolWorkerData->aabbCache, false);
	if (poolWorkerData->rebuildDynQuadTrees) {
		jobTagQtreeDyn = jobManager.addJob(&dynqTreeJob);
	}

	BuildQtreeJob particleTreeJob = BuildQtreeJob(world, poolWorkerData->dynCollidables, poolWorkerData->qtreeParticle, poolWorkerData->aabbCache, true);
	int jobTagQtreeParticle = jobManager.addJob(&particleTreeJob);

	if (poolWorkerData->rebuildDynQuadTrees) {	// we must wait for this job to finish before insertein sensors into dyn treee
		jobManager.waitFor(jobTagQtreeDyn);
		int jobTagQtreeDynSensor = -1;
		BuildQtreeJob dynSensorqTreeJob = BuildQtreeJob(world, poolWorkerData->sensorCollidables, poolWorkerData->qtreeDynamic, poolWorkerData->aabbCache, false);
		jobTagQtreeDynSensor = jobManager.addJob(&dynSensorqTreeJob);
		jobManager.waitFor(jobTagQtreeDynSensor);
	}

	if (poolWorkerData->rebuildStatQuadTrees)
		jobManager.waitFor(jobTagStaticRebuild);
	jobManager.waitFor(jobTagQtreeParticle);
}

void CollisionSystem::cleanBuffers(World& world)
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
	if (poolWorkerData->collisionResponses.size() < world.maxEntityIndex()) poolWorkerData->collisionResponses.resize(world.maxEntityIndex());
	cleanAndShrink(poolWorkerData->aabbCache);
	if (poolWorkerData->aabbCache.size() < world.maxEntityIndex()) poolWorkerData->aabbCache.resize(world.maxEntityIndex());
	for (auto& split : poolWorkerData->collisionInfoBuffers) cleanAndShrink(split);
	cleanAndShrink(collisionInfos);
	collisionInfoBegins.clear();
	collisionInfoEnds.clear();
	cleanAndShrink(collisionInfos);
	collisionInfoBegins.clear();
	collisionInfoEnds.clear();

	cleanAndShrink(dynCheckJobs);
	cleanAndShrink(dynTags);
	cleanAndShrink(sensorCheckJobs);
	cleanAndShrink(sensorTags);
}

void CollisionSystem::collisionDetection(World& world)
{
	Timer t3(perfLog.getInputRef("collisionnarrow"));

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

	jobManager.waitAndHelp(&dynTags);
	jobManager.waitAndHelp(&sensorTags);

	t3.stop();
	Timer t4(perfLog.getInputRef("collisionpost"));

	// reset quadtree rebuild flags
	poolWorkerData->rebuildDynQuadTrees = false;
	poolWorkerData->rebuildStatQuadTrees = false;

	// store all collisioninfos in one vectorcapacity
	for (auto const& collisionInfosplit : poolWorkerData->collisionInfoBuffers) {
		collisionInfos.insert(collisionInfos.end(), collisionInfosplit.begin(), collisionInfosplit.end());
	}

	// build hashtables for first and last iterator element of collisioninfo
	uint32_t lastIDA{};
	for (auto iter = collisionInfos.begin(); iter != collisionInfos.end(); ++iter) {
		if (iter == collisionInfos.begin()) {	//initialize values from first element
			lastIDA = iter->indexA;
			collisionInfoBegins.insert({ iter->indexA, iter });
		}
		if (lastIDA != iter->indexA) {	//new idA found
			collisionInfoEnds.insert({ lastIDA, iter });
			collisionInfoBegins.insert({ iter->indexA, iter });
			lastIDA = iter->indexA;	//set lastId to new id
		}
	}
	collisionInfoEnds.insert({ lastIDA, collisionInfos.end() });

#ifdef DEBUG_QTREE_FINDPLAYER
	Entity player;
	for (auto p : world.entity_view<Player>())
		player = p;

	std::vector<Entity> near;
	for (auto ent : world.entity_view<Collider, Movement>()) {
		near.clear();
		poolWorkerData->qtreeDynamic.querry(near, PosSize(world.getComp<Base>(ent).position, poolWorkerData->aabbCache.at(ent)));
		for (auto oent : near) {
			if (oent == player) {
				auto pos = world.getComp<Base>(ent).position;
				auto d = Drawable(0, pos, 0.99, Vec2(0.1,0.1), Vec4(0, 0, 1, 1), Form::Circle, 0);
				debugDrawables.push_back(d);
			}
		}
	}
#endif
}
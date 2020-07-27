#pragma once

#include <mutex>
#include <sstream>
#include <condition_variable>

#include "robin_hood.h"

#include "Physics.hpp"
#include "collision_detection.hpp"
#include "QuadTree.hpp"
#include "Timing.hpp"
#include "World.hpp"

//Commit

struct CollisionSyncData {
	CollisionSyncData() : run{ true }, mut{}, cond{}, mut2{}, cond2{}, insertReady{0}  {

	}
	bool run;

	//phase 1 sync
	std::mutex mut;
	std::condition_variable cond;

	std::vector<bool> go;


	// phase 2 sync
	std::mutex mut2;
	std::condition_variable cond2;

	int insertReady;
};

struct CollisionPoolData {
	CollisionPoolData(size_t qtreeCapacity, World& world) :
		world{ world },
		qtreeDynamic(0, 0, qtreeCapacity, world),
		qtreeStatic(0, 0, qtreeCapacity, world)
	{}

	World& world;
	std::vector<uint32_t> sensorCollidables;
	std::vector<uint32_t> dynCollidables;
	std::vector<uint32_t> statCollidables;
	std::vector<CollisionResponse> collisionResponses;
	std::vector<Vec2> aabbCache;
	bool rebuildDynQuadTrees = true;
	Quadtree2 qtreeDynamic;
	bool rebuildStatQuadTrees = true;
	Quadtree2 qtreeStatic;

	GridPhysics<bool> staticCollisionGrid;

	std::vector<Drawable> debugDrawables;
};

struct CollisionWorkerData {
	int id = 0;
	uint32_t beginSensor;
	uint32_t endSensor;
	uint32_t beginDyn;
	uint32_t endDyn;
	uint32_t beginStat;
	uint32_t endStat;

	std::vector<IndexCollisionInfo>* collisionInfos;
};

struct CollisionWorker {
	CollisionWorker(std::shared_ptr<CollisionWorkerData> workerData, std::shared_ptr<CollisionPoolData> poolData, std::shared_ptr<CollisionSyncData> syncData_, unsigned threadCount_) :
		workerData{ workerData }, syncData{ syncData_ }, poolData{ poolData }, physicsThreadCount{ threadCount_ }
	{}
	std::shared_ptr<CollisionWorkerData> workerData;
	std::shared_ptr<CollisionPoolData> poolData;
	std::shared_ptr<CollisionSyncData> syncData;

	std::vector<uint32_t> nearCollidablesBuffer;	// reuse heap memory for all dyn collidable collisions

	unsigned const physicsThreadCount;
	bool run{ true };

	void cacheAABBs(std::vector<entity_index_type>& colliders);

	void waitForUpdate();

	void waitForOtherWorkers();

	void collisionFunction(entity_index_type collID, Quadtree2 const& quadtree, bool dynamic);

	//void updateStaticGrid();

	void operator()(); 
};
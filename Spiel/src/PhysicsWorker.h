#pragma once

#include <mutex>
#include <sstream>
#include <condition_variable>

#include "Physics.h"
#include "collision_detection.h"
#include "robin_hood.h"
#include "QuadTree.h"
#include "Timing.h"
#include "World.h"

struct PhysicsSharedSyncData {
	PhysicsSharedSyncData() : run{ true }, mut{}, cond{}, mut2{}, cond2{}, insertReady{0}  {

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

struct PhysicsPoolData {
	PhysicsPoolData(World& wrld, size_t qtreeCapacity) :
		world{ wrld },
		qtreeDynamic(0, 0, qtreeCapacity, wrld),
		qtreeStatic(0, 0, qtreeCapacity, wrld)
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

struct PhysicsPerThreadData {
	int id = 0;
	uint32_t beginSensor;
	uint32_t endSensor;
	uint32_t beginDyn;
	uint32_t endDyn;
	uint32_t beginStat;
	uint32_t endStat;

	std::vector<CollisionInfo>* collisionInfos;
};

struct PhysicsWorker {
	PhysicsWorker(std::shared_ptr<PhysicsPerThreadData> physicsData_, std::shared_ptr<PhysicsPoolData> poolData, std::shared_ptr<PhysicsSharedSyncData> syncData_, unsigned threadCount_) :
		physicsData{ physicsData_ }, syncData{ syncData_ }, poolData{ poolData }, physicsThreadCount{ threadCount_ }
	{}
	std::shared_ptr<PhysicsPerThreadData> physicsData;
	std::shared_ptr<PhysicsPoolData> poolData;
	std::shared_ptr<PhysicsSharedSyncData> syncData;

	std::vector<uint32_t> nearCollidablesBuffer;	// reuse heap memory for all dyn collidable collisions

	unsigned const physicsThreadCount;
	bool run{ true };

	void cacheAABBs(std::vector<entity_index_type>& colliders);

	void waitForUpdate();

	void waitForOtherWorkers();

	void collisionFunction(entity_index_type collID, Quadtree2 const& quadtree, bool dynamic);

	void updateStaticGrid();

	void operator()(); 
};
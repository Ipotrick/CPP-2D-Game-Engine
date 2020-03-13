#pragma once

#include <mutex>
#include <sstream>
#include <condition_variable>

#include "Physics.h"
#include "QuadTree.h"
#include "Timing.h"

struct PhysicsSyncData {
	PhysicsSyncData() : run{ true }, mut{}, cond{}, mut2{}, cond2{}, insertReady{0}  {}
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

struct PhysicsSharedData {
	int id = 0;
	std::vector<std::pair<uint32_t, Collidable*>>* dynCollidables;
	int beginDyn;
	int endDyn;
	std::vector<std::pair<uint32_t, Collidable*>>* statCollidables;
	int beginStat;
	int endStat;
	float deltaTime;
	std::vector<CollisionResponse> * collisionResponses;
	std::vector<CollisionInfo> * collisionInfos;
	std::vector<Quadtree>* qtrees;
};

struct PhysicsWorker {
	PhysicsWorker(std::shared_ptr<PhysicsSharedData> physicsData_, std::shared_ptr<PhysicsSyncData> syncData_, unsigned threadCount_) :
		physicsData{ physicsData_}, syncData{ syncData_ }, physicsThreadCount{ threadCount_ }
	{}
	std::shared_ptr<PhysicsSharedData> physicsData;
	std::shared_ptr<PhysicsSyncData> syncData;

	unsigned const physicsThreadCount;

	void operator()(); 
};
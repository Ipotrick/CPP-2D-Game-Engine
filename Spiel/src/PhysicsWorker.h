#pragma once

#include <mutex>
#include <sstream>
#include <condition_variable>

#include "Physics.h"
#include "QuadTree.h"

struct PhysicsSyncData {
	PhysicsSyncData() : mut{}, cond{}, run{ true }  {}
	std::mutex mut;
	std::condition_variable cond;
	bool run;

	std::vector<bool> go;
};

struct PhysicsSharedData {
	int id = 0;
	std::vector<Collidable*> * dynCollidables;
	int begin;
	int end;
	Quadtree * qtree;
	std::vector<CollisionResponse> * collisionResponses;
	std::vector<CollisionInfo> * collisionInfos;
};

struct PhysicsWorker {
	PhysicsWorker(std::shared_ptr<PhysicsSharedData> physicsData_, std::shared_ptr<PhysicsSyncData> syncData_) :
		physicsData{ physicsData_}, syncData{ syncData_ }
	{}
	std::shared_ptr<PhysicsSharedData> physicsData;
	std::shared_ptr<PhysicsSyncData> syncData;

	void operator()(); 
};
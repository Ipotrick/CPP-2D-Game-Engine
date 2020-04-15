#pragma once

#include <mutex>
#include <sstream>
#include <condition_variable>

#include "Physics.h"
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
	World* world;
	std::vector<uint32_t>* sensorCollidables;
	std::vector<uint32_t>* dynCollidables;
	std::vector<uint32_t>* statCollidables;
	std::vector<CollisionResponse>* collisionResponses;
	bool rebuildDynQuadTrees = true;
	std::shared_ptr<std::vector<Quadtree2>> qtreesDynamic;
	bool rebuildStatQuadTrees = true;
	std::shared_ptr<std::vector<Quadtree2>> qtreesStatic;
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
		physicsData{ physicsData_ }, syncData{ syncData_ }, physicsPoolData{ poolData }, physicsThreadCount{ threadCount_ }
	{}
	std::shared_ptr<PhysicsPerThreadData> physicsData;
	std::shared_ptr<PhysicsPoolData> physicsPoolData;
	std::shared_ptr<PhysicsSharedSyncData> syncData;

	unsigned const physicsThreadCount;

	void operator()(); 

	bool areEntsRelated(ent_id_t endA, ent_id_t endB);
};

inline bool PhysicsWorker::areEntsRelated(ent_id_t collID, ent_id_t otherID) {
	if (physicsPoolData->world->hasComp<Slave>(collID) && physicsPoolData->world->hasComp<Slave>(otherID)) {	//same owner no collision check
		if (physicsPoolData->world->getComp<Slave>(collID).ownerID == physicsPoolData->world->getComp<Slave>(otherID).ownerID) {
			return true;
		}
	}
	else if (physicsPoolData->world->hasComp<Slave>(collID)) {
		if (physicsPoolData->world->getComp<Slave>(collID).ownerID == otherID) {
			return true;
		}
	}
	else if (physicsPoolData->world->hasComp<Slave>(otherID)) {
		if (physicsPoolData->world->getComp<Slave>(otherID).ownerID == collID) {
			return true;
		}
	}
	return false;
}
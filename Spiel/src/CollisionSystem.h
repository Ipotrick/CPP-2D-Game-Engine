#pragma once

#include <vector>

#include "robin_hood.h"

#include "vector_math.h"
#include "collision_detection.h"
#include "Physics.h"
#include "CoreSystem.h"
#include "CollisionWorker.h"
#include "Perf.h"
#include "JobManager.hpp"

class CollisionSystem : public CoreSystem {
	friend class PhysicsSystem;
public:
	CollisionSystem(World& world, uint32_t threadCount, PerfLogger& perfLog, float staticGridResolution = 0.5f, uint32_t qtreeCapacity = 6);
	void execute(World& world, float deltaTime);
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisions(entity_index_type id_);
	std::vector<CollisionInfo>& getAllCollisions();
	GridPhysics<bool> getStaticGrid();
	void end();
public:
	std::vector<Drawable> debugDrawables;
	JobManager jobManager;
private:
	void prepare();
	void cleanBuffers();
	void collisionDetection();
private:
	PerfLogger& perfLog;
	uint32_t const threadCount;
	uint32_t qtreeCapacity;
	bool rebuildStaticData;

	std::vector<std::thread> workerThreads;
	std::shared_ptr<CollisionSyncData> syncWorkerData;
	std::shared_ptr<CollisionPoolData> poolWorkerData;
	std::vector<std::shared_ptr<CollisionWorkerData>> perWorkerData;
	std::vector<std::vector<entity_index_type>> neighborBuffer;

	// buffers
	std::vector<std::vector<CollisionInfo>> collisionInfosSplit;
	std::vector<CollisionInfo> collisionInfos{};
	robin_hood::unordered_map<entity_index_type, std::vector<CollisionInfo>::iterator> collisionInfoBegins;
	robin_hood::unordered_map<entity_index_type, std::vector<CollisionInfo>::iterator> collisionInfoEnds;
};
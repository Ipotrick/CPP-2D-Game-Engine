#pragma once

#include <vector>

#include "robin_hood.h"

#include "vector_math.hpp"
#include "collision_detection.hpp"
#include "Physics.hpp"
#include "CoreSystem.hpp"
#include "CollisionWorkerData.hpp"
#include "Perf.hpp"
#include "JobManager.hpp"
#include "CollisionCheckJob.hpp"

class CollisionSystem : public CoreSystem {
	friend class PhysicsSystem;
public:
	CollisionSystem(World& world, JobManager& jobManager, PerfLogger& perfLog, float staticGridResolution = 0.5f, uint32_t qtreeCapacity = 6);
	void execute(World& world, float deltaTime);
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisions(entity_index_type id_);
	std::vector<CollisionInfo>& getAllCollisions();
	GridPhysics<bool> getStaticGrid();
	void end();
public:
	std::vector<Drawable> debugDrawables;
	JobManager& jobManager;
private:
	void prepare();
	void cleanBuffers();
	void collisionDetection();
private:
	// constants:
	const int jobMaxEntityCount = 50;
	uint32_t qtreeCapacity;

	PerfLogger& perfLog;
	bool rebuildStaticData;

	// buffers
	std::shared_ptr<CollisionPoolData> poolWorkerData;
	std::vector<CollisionInfo> collisionInfos{};
	robin_hood::unordered_map<entity_index_type, std::vector<CollisionInfo>::iterator> collisionInfoBegins;
	robin_hood::unordered_map<entity_index_type, std::vector<CollisionInfo>::iterator> collisionInfoEnds;

	// buffers for jobs:
	std::vector<DynCollisionCheckJob> dynCheckJobs;
	std::vector<uint32_t> dynTags;
	std::vector<SensorCollisionCheckJob> sensorCheckJobs;
	std::vector<uint32_t> sensorTags;
};
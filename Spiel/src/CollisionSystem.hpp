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

class CollisionSystem {
	friend class PhysicsSystem;
public:
	CollisionSystem(World& world, JobManager& jobManager, PerfLogger& perfLog, float staticGridResolution = 0.5f, uint32_t qtreeCapacity = 30);
	void execute(World& world, float deltaTime);
	std::tuple<std::vector<IndexCollisionInfo>::iterator, std::vector<IndexCollisionInfo>::iterator> getCollisions(Entity id_);
	std::vector<IndexCollisionInfo>& getAllCollisions();
	GridPhysics<bool> getStaticGrid();
	void end();
public:
	std::vector<Drawable> debugDrawables;
	JobManager& jobManager;
private:
	void prepare(World& world);
	void cleanBuffers(World& world);
	void collisionDetection(World& world);
private:
	// constants:
	const int jobMaxEntityCount = 200;
	uint32_t qtreeCapacity;

	PerfLogger& perfLog;
	bool rebuildStaticData;

	// buffers
	std::shared_ptr<CollisionPoolData> poolWorkerData;
	std::vector<IndexCollisionInfo> collisionInfos{};
	robin_hood::unordered_map<Entity, std::vector<IndexCollisionInfo>::iterator> collisionInfoBegins;
	robin_hood::unordered_map<Entity, std::vector<IndexCollisionInfo>::iterator> collisionInfoEnds;

	// buffers for jobs:
	std::vector<DynCollisionCheckJob> dynCheckJobs;
	std::vector<Tag> dynTags;
	std::vector<SensorCollisionCheckJob> sensorCheckJobs;
	std::vector<Tag> sensorTags;
};
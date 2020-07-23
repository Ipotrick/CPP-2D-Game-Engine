#pragma once

#include <vector>
#include <queue>
#include <set>

#include "robin_hood.h"

#include "vector_math.hpp"
#include "collision_detection.hpp"
#include "Physics.hpp"
#include "CoreSystem.hpp"
#include "Perf.hpp"

#include "CollisionSystem.hpp"
#include "IndexSet.hpp"
#include "CollisionResolutionJob.hpp"

//#define DEBUG_COLLIDER_SLEEP

class PhysicsSystem : public CoreSystem {
public:
	PhysicsSystem(World& world, JobManager& jobManager, PerfLogger& perfLog);
	void execute(World& world, float deltaTime, CollisionSystem& collSys);
	void end();
public:
	std::vector<Drawable> debugDrawables;
private: 
	void findIslands(float deltaTime, CollisionSystem& collSys);
	void applyPhysics(float deltaTime, CollisionSystem& collSys);

	void propagateChildPushoutToParent(CollisionSystem& collSys);
	void syncBaseChildrenToParents();
private:
	JobManager& jobManager;
	const int impulseResulutionIterations = 10;
	const int impulseResolutionJobCount = 80;
	PerfLogger& perfLog;

	//buffers:
	std::vector<float> overlapAccumBuffer;
	std::vector<Vec2> velocityBuffer;

	std::vector<int> entityIslandMarks;
	std::vector<std::vector<IndexCollisionInfo>> collInfoIslands;
	std::vector<IndexCollisionInfo> collInfoIslandsBorder; 
	std::vector<std::pair<int, int>> islandBatches;


	std::vector<CollisionResolutionJob> resolutionJobs;
	std::vector<int> resolutionJobTags;
};
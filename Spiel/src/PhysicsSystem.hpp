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
#include "ImpulseResolutionJob.hpp"

//#define DEBUG_COLLIDER_SLEEP
//#define DEBUG_PRESSURE
//#define DEBUG_ISLANDS

class PhysicsSystem {
public:
	PhysicsSystem(JobManager& jobManager, PerfLogger& perfLog);
	void execute(World& world, float deltaTime, CollisionSystem& collSys);
	void end();
public:
	std::vector<Drawable> debugDrawables;
private: 
	void findIslands(CollisionSystem& collSys, World& world);
	void applyPhysics(float deltaTime, CollisionSystem& collSys, World& world);

	void propagateChildPushoutToParent(CollisionSystem& collSys, World& world);
	void syncBaseChildrenToParents(World& world);
private:
	JobManager& jobManager;
	const int impulseResulutionIterations = 15;		// number of iterations of impulse propagation per frame									(recommendet value = 3 with low forces to 15 with high forces)
	const int maxIslandSize = 200;					// maximum size (number of entities) in an collision island									(recommendet value = 200)
	const int islandMergeIterations = 2;			// amount if iterations used to merge collision islands (more iterations=> smaller border)	(recommendet value = 2)
	const int impulseResolutionMaxBatchSize = 400;	// if a single island has more collisions than the maximum, it will get it's very own batch (recommendet value = 300)
	PerfLogger& perfLog;

	//buffers:
	std::vector<float> overlapAccumBuffer;	// semi pressure value for entities. Hierher overlapAccum => more resistant to overlap
	std::vector<Vec2> velocityBuffer;		// buffer used by the overlap reolution job

	std::vector<int> entityToIsland;					
	std::vector<std::vector<IndexCollisionInfo>> collInfoIslands;	// buffer for collisionInfos of the islands
	std::vector<IndexCollisionInfo> collInfoIslandsBorder;			// buffer for collisionInfos on the borders
	std::vector<std::pair<int, int>> islandBatches;					// islands are grouped into batches if they only contain a small amout of collisions
	std::vector<int> islandSizes;									// sizes(amount of entities) of islands. THis is used in the calculation of islands

	std::vector<ImpulseResolutionJob> resolutionJobs;	
	std::vector<int> resolutionJobTags;
};
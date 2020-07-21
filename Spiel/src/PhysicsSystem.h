#pragma once

#include <vector>

#include "robin_hood.h"

#include "vector_math.h"
#include "collision_detection.h"
#include "Physics.h"
#include "CoreSystem.h"
#include "Perf.h"

#include "CollisionSystem.h"

//#define DEBUG_COLLIDER_SLEEP

class PhysicsSystem : public CoreSystem {
public:
	PhysicsSystem(World& world, JobManager& jobManager, PerfLogger& perfLog);
	void execute(World& world, float deltaTime, CollisionSystem& collSys);
	void end();
public:
	std::vector<Drawable> debugDrawables;
private: 
	void applyPhysics(float deltaTime, CollisionSystem& collSys);

	void propagateChildPushoutToParent(CollisionSystem& collSys);
	void syncBaseChildrenToParents();
private:
	JobManager& jobManager;
	const int impulseResulutionIterations = 25;
	PerfLogger& perfLog;

	//buffers:
	std::vector<float> overlapAccumBuffer;
	std::vector<Vec2> velocityBuffer;
};
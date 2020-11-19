#pragma once

#include <stack>

#include "vector_math.hpp"
#include "collision_detection.hpp"
#include "Physics.hpp"
#include "CoreSystem.hpp"
#include "Perf.hpp"
#include "CollisionConstraintSet.hpp"
#include "CollisionSystem.hpp"
#include "JobManager.hpp"
#include "Physics.hpp"
/*
* NEXT TASK TODO:
* implement fake movement
* implement new position correction based on fake movement
* implement settings for iteration count of force and penetratin constraints
*/

class PhysicsSystem2 {
public:
	PhysicsSystem2(JobManager& jobs);
	void execute(World& world, float deltaTime, CollisionSystem& collSys);
	const std::vector<Drawable>& getDebugDrawables() const;
private:
	std::vector<Drawable> debugDrawables;
	void updateCollisionConstraints(World& world, CollisionSystem& collSys);
	void eraseDeadConstraints();
	void prepareConstraints(World& world, float deltaTime);
	void springyPositionCorrection(World& world, float deltaTime);
	void applyImpulse(World& world, CollisionConstraint& c);
	void applyImpulses(World& world);
	void applyImpulsesMultiThreadded(World& world);
	void applyForcefields(World& world, float deltaTime);
	void drawAllCollisionConstraints();

	bool positionCorrection = true;
	bool accumulateImpulses = true;
	bool warmStart = true;
	float minDelaTime = 0.2f;		// if deltaTime is bigger, the simulation will slow down to maintain precision
	int impulseIterations = 15;

	JobManager& jobManager;

	CollisionConstraintSet collConstraints;

	std::vector<bool> visited;
	std::vector<bool> used;
	const int MAX_LAYERS = 10;
	const int CONSTRAINTS_PER_JOB = 5000;
	std::vector<std::vector<CollisionConstraint*>> disjointPairs;
	std::vector<CollisionConstraint*> restConstraints;
};
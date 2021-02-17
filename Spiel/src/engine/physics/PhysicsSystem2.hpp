#pragma once

#include "../../engine/math/vector_math.hpp"
#include "../collision/collision_detection.hpp"
#include "Physics.hpp"
#include "../../engine/util/Perf.hpp"
#include "CollisionConstraintSet.hpp"
#include "../collision/CollisionSystem.hpp"
#include "../../engine/util/debug.hpp"
#include "../../engine/util/Log.hpp"
#include "../collision/CoreSystemUniforms.hpp"

/**
* NEXT TASK TODO:
* implement fake movement
* implement new position correction based on fake movement
* implement settings for iteration count of force and penetratin constraints
*/

class PhysicsSystem2 {
public:
	PhysicsSystem2();
	void execute(CollisionSECM world, PhysicsUniforms const& uniform, float deltaTime, CollisionSystem& collSys);
	const std::vector<Sprite>& getDebugSprites() const;
private:
	std::vector<Sprite> debugSprites;
	void updateCollisionConstraints(CollisionSECM world, CollisionSystem& collSys);
	void eraseDeadConstraints();
	void prepareConstraints(CollisionSECM world, float deltaTime);
	void springyPositionCorrection(CollisionSECM world, float deltaTime);
	void applyImpulse(CollisionSECM world, CollisionConstraint& c);
	void applyImpulses(CollisionSECM world);
	void applyForcefields(CollisionSECM world, PhysicsUniforms const& uniform, float deltaTime);
	void drawAllCollisionConstraints();

	/* EXPERIMENTAL */
	void clearDuplicates(CollisionSECM world, CollisionSystem& collSys);
	/* EXPERIMENTAL */
	void findIslands(CollisionSECM world, CollisionSystem& collSys);

	bool positionCorrection = true;
	bool accumulateImpulses = true;
	bool warmStart = true;
	float minDelaTime = 0.2f;		// if deltaTime is bigger, the simulation will slow down to maintain precision
	int impulseIterations = 15;

	CollisionConstraintSet collConstraints;

	std::vector<CollisionInfo> uniqueCollisionInfos; 
	robin_hood::unordered_set<uint64_t> uniqueCollisionInfosSetBuffer;

	std::vector<bool> visited;
	std::vector<bool> used;
	const int MAX_LAYERS = 10;
	const int CONSTRAINTS_PER_JOB = 5000;
	std::vector<std::vector<CollisionConstraint*>> disjointPairs;
	std::vector<CollisionConstraint*> restConstraints;
};

#define LOG_FUNCTION_TIME(message, function) \
{ \
	std::chrono::microseconds elapsedTime; \
	{ \
		Timer t(elapsedTime); \
		function; \
	} \
	std::cout << message << ", time taken: " << elapsedTime.count() << "mics" << std::endl; \
}
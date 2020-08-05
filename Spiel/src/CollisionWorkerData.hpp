#pragma once
#include <mutex>
#include <sstream>
#include <condition_variable>

#include "Physics.hpp"
#include "collision_detection.hpp"
#include "robin_hood.h"
#include "QuadTree.hpp"
#include "Timing.hpp"
#include "World.hpp"
#include "SweepAndPrune.hpp"

struct CollisionPoolData {
	CollisionPoolData(World& wrld, size_t qtreeCapacity, int workerCount) :
		world{ wrld },
		qtreeDynamic(0, 0, qtreeCapacity, wrld),
		qtreeStatic(0, 0, qtreeCapacity, wrld),
		qtreeParticle(0,0, qtreeCapacity, wrld)
	{
		for (int i = 0; i < workerCount; i++) {
			nearCollidablesBuffers.push_back(std::vector<Entity>());
			collisionInfoBuffers.push_back(std::vector<IndexCollisionInfo>());
		}
	}

	SAPBuffer* sapBuffer;
	World& world;
	std::vector<uint32_t> sensorCollidables;
	std::vector<uint32_t> dynCollidables;
	std::vector<uint32_t> statCollidables;
	std::vector<CollisionResponse> collisionResponses;
	std::vector<Vec2> aabbCache;
	bool rebuildDynQuadTrees = true;
	Quadtree2 qtreeDynamic;
	bool rebuildStatQuadTrees = true;
	Quadtree2 qtreeStatic;
	Quadtree2 qtreeParticle;

	std::vector<std::vector<Entity>> nearCollidablesBuffers;
	std::vector<std::vector<IndexCollisionInfo>> collisionInfoBuffers;

	GridPhysics<bool> staticCollisionGrid;

	std::vector<Drawable> debugDrawables;
};
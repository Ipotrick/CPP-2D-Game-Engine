#pragma once
#include "JobManager.hpp"
#include "EntityComponentManager.hpp"
#include "QuadTree.hpp"
#include "collision_detection.hpp"
#include "Physics.hpp"
#include "robin_hood.h"

class CollisionCheckJobBuffers {
public:
	CollisionCheckJobBuffers(size_t workerNum)
		: workerNum{ workerNum }
	{
		for (int i = 0; i < workerNum; i++) {
			nearEntities.push_back(std::vector<Entity>());
			collisionInfos.push_back(std::vector<CollisionInfo>());
		}
	}
	void clear()
	{
		for (int i = 0; i < workerNum; i++) {
			nearEntities.at(i).clear();
			collisionInfos.at(i).clear();
		}
	}
	std::vector<std::vector<Entity>> nearEntities;
	std::vector<std::vector<CollisionInfo>> collisionInfos;
	const size_t workerNum;
};

class CollisionCheckJob : public JobFunctor {
public:
	explicit CollisionCheckJob(
		EntityComponentManager& manager,
		CollisionCheckJobBuffers& poolData,
		const std::vector<Entity>& entities,
		const Quadtree& qtree1,
		const Quadtree& qtree2,
		const Quadtree& qtree3,
		const Quadtree& qtree4,
		const uint8_t qtreeMask,
		const std::vector<Vec2>& aabbCache)
		:manager{ manager }, poolData{ poolData }, entities{ entities },
		qtree1{ qtree1 },
		qtree2{ qtree2 },
		qtree3{ qtree3 },
		qtree4{ qtree4 },
		QTREE_MASK{ qtreeMask },
		aabbCache{ aabbCache }
	{ }

	void execute(int workerId) override
	{
		for (Entity i = 0; i < entities.size(); i++) {
			Collider& coll = manager.getComp<Collider>(entities.at(i));
			if (!coll.sleeping) {
				if (qtree1.IGNORE_TAG & QTREE_MASK && !coll.isIgnoring(qtree1.IGNORE_TAG))
					collisionFunction(entities.at(i), qtree1, poolData.nearEntities.at(workerId), poolData.collisionInfos.at(workerId));
				if (qtree2.IGNORE_TAG & QTREE_MASK && !coll.isIgnoring(qtree2.IGNORE_TAG))
					collisionFunction(entities.at(i), qtree2, poolData.nearEntities.at(workerId), poolData.collisionInfos.at(workerId));
				if (qtree3.IGNORE_TAG & QTREE_MASK && !coll.isIgnoring(qtree3.IGNORE_TAG))
					collisionFunction(entities.at(i), qtree3, poolData.nearEntities.at(workerId), poolData.collisionInfos.at(workerId));
				if (qtree4.IGNORE_TAG & QTREE_MASK && !coll.isIgnoring(qtree4.IGNORE_TAG))
					collisionFunction(entities.at(i), qtree4, poolData.nearEntities.at(workerId), poolData.collisionInfos.at(workerId));
			}
		}
	}
protected:
	EntityComponentManager& manager;
	CollisionCheckJobBuffers& poolData;
	const std::vector<Entity>& entities;
	const Quadtree& qtree1;
	const Quadtree& qtree2;
	const Quadtree& qtree3;
	const Quadtree& qtree4;
	const uint8_t QTREE_MASK;
	const std::vector<Vec2>& aabbCache;
	std::vector<Entity> near;
	std::vector<CollPoint> collisionVertices;

	void collisionFunction(
		const Entity collID,
		const Quadtree& qtree,
		std::vector<Entity>& nearCollidablesBuffer,
		std::vector<CollisionInfo>& collisionInfos);
};
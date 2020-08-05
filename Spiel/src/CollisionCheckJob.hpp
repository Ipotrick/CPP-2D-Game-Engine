#pragma once
#include "JobManager.hpp"
#include "EntityComponentManager.hpp"
#include "QuadTree.hpp"
#include "collision_detection.hpp"
#include "Physics.hpp"
#include "CollisionWorkerData.hpp"
#include "robin_hood.h"

class CollisionCheckJob : public JobFunctor {
protected:
	std::shared_ptr<CollisionPoolData> poolData;
	std::vector<Entity>& entities;
	uint32_t begin = 0;
	uint32_t end = 0;

	void collisionFunction(
		const Entity collID,
		const bool otherDynamic,
		const Quadtree2& qtree,
		std::vector<Entity>& nearCollidablesBuffer,
		std::vector<IndexCollisionInfo>& collisionInfos);
public:
	CollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<Entity>& entities,
		uint32_t begin,
		uint32_t end)
		:poolData{ poolData }, entities{ entities }, begin{ begin }, end{ end }
	{
	}

	void execute(int workerId) override = 0;
};

class DynCollisionCheckJob : public CollisionCheckJob {
public:
	DynCollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<Entity>& entities,
		uint32_t begin,
		uint32_t end)
		:CollisionCheckJob(poolData, entities, begin, end)
	{}

	void execute(int workerId) override {
		for (uint32_t i = begin; i < end; i++) {
			auto& collID = entities.at(i);
			collisionFunction(collID, true, poolData->qtreeDynamic, poolData->nearCollidablesBuffers.at(workerId), poolData->collisionInfoBuffers.at(workerId));
			collisionFunction(collID, false, poolData->qtreeStatic, poolData->nearCollidablesBuffers.at(workerId), poolData->collisionInfoBuffers.at(workerId));
		}
	}
};

class SensorCollisionCheckJob : public CollisionCheckJob {
public:
	SensorCollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<Entity>& entities,
		uint32_t begin,
		uint32_t end)
		:CollisionCheckJob(poolData, entities, begin, end)
	{}

	void execute(int workerId) override {
		for (uint32_t i = begin; i < end; i++) {
			auto& collID = entities.at(i);
			collisionFunction(collID, true, poolData->qtreeDynamic, poolData->nearCollidablesBuffers.at(workerId), poolData->collisionInfoBuffers.at(workerId));
			collisionFunction(collID, true, poolData->qtreeParticle, poolData->nearCollidablesBuffers.at(workerId), poolData->collisionInfoBuffers.at(workerId));
			if (poolData->world.hasntComps<LinearEffector, FrictionEffector>(collID)) {
				collisionFunction(collID, false, poolData->qtreeStatic, poolData->nearCollidablesBuffers.at(workerId), poolData->collisionInfoBuffers.at(workerId));
			}
		}
	}
};

inline void CollisionCheckJob::collisionFunction(
	const Entity collID, 
	const bool otherDynamic,
	const Quadtree2& qtree,
	std::vector<Entity>& nearCollidablesBuffer,
	std::vector<IndexCollisionInfo>& collisionInfos)
{
	auto& manager = poolData->world;
	auto& aabbCache = poolData->aabbCache;
	auto& collisionResponses = poolData->collisionResponses;

	auto& colliderColl = manager.getComp<Collider>(collID);
	auto& baseColl = manager.getComp<Base>(collID);

	if (!colliderColl.sleeping) {
		const CollidableAdapter collAdapter = CollidableAdapter(
			baseColl.position,
			baseColl.rotation,
			colliderColl.size,
			colliderColl.form,
			true,
			baseColl.rotaVec);

		const PosSize posSize(baseColl.position, poolData->aabbCache.at(collID));

		nearCollidablesBuffer.clear();
		qtree.querry(nearCollidablesBuffer, posSize);

		for (auto& otherID : nearCollidablesBuffer) {
			if (collID != otherID) { //do not check against self
				const auto baseOther = manager.getComp<Base>(otherID);
				const auto colliderOther = manager.getComp<Collider>(otherID);
				if (!(colliderColl.collisionMaskAgainst & colliderOther.collisionMaskSelf)) {

					CollidableAdapter otherAdapter(
						baseOther.position,
						baseOther.rotation,
						colliderOther.size,
						colliderOther.form,
						otherDynamic,
						baseOther.rotaVec);

					const auto newTestResult = collisionTestCachedAABB(collAdapter, otherAdapter, aabbCache.at(collID), aabbCache.at(otherID));
					if (newTestResult.collided) {
						if (!manager.areRelated(collID, otherID)) {
							collisionInfos.push_back(IndexCollisionInfo(collID, otherID, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionPos));
						}
					}
				}
			}
		}
	}
}
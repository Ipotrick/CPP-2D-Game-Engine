#pragma once
#include "JobManager.hpp"
#include "EntityComponentManager.h"
#include "QuadTree.h"
#include "collision_detection.h"
#include "Physics.h"
#include "CollisionWorkerData.hpp"

class CollisionCheckJob : public JobFunctor {
protected:
	std::shared_ptr<CollisionPoolData> poolData;
	std::vector<entity_index_type>& entities;
	uint32_t begin = 0;
	uint32_t end = 0;

	void collisionFunction(
		entity_index_type collID,
		bool otherDynamic,
		Quadtree2& qtree,
		std::vector<entity_index_type>& nearCollidablesBuffer,
		std::vector<CollisionInfo>& collisionInfos);
public:
	CollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<entity_index_type>& entities,
		uint32_t begin,
		uint32_t end)
		:poolData{ poolData }, entities{ entities }, begin{ begin }, end{ end }
	{}

	int operator()(int workerId) override = 0;
};

class DynCollisionCheckJob : public CollisionCheckJob {
public:
	DynCollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<entity_index_type>& entities,
		uint32_t begin,
		uint32_t end)
		:CollisionCheckJob(poolData, entities, begin, end)
	{}

	int operator()(int workerId) override {
		for (uint32_t i = begin; i < end; i++) {
			auto& collID = entities.at(i);
			collisionFunction(collID, true, poolData->qtreeDynamic, poolData->nearCollidablesBuffers[workerId], poolData->collisionInfoBuffers[workerId]);
			collisionFunction(collID, false, poolData->qtreeStatic, poolData->nearCollidablesBuffers[workerId], poolData->collisionInfoBuffers[workerId]);
		}
		return 0;
	}
};

class SensorCollisionCheckJob : public CollisionCheckJob {
public:
	SensorCollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<entity_index_type>& entities,
		uint32_t begin,
		uint32_t end)
		:CollisionCheckJob(poolData, entities, begin, end)
	{}

	int operator()(int workerId) override {
		for (uint32_t i = begin; i < end; i++) {
			auto& collID = entities.at(i);
			collisionFunction(collID, true, poolData->qtreeDynamic, poolData->nearCollidablesBuffers[workerId], poolData->collisionInfoBuffers[workerId]);
			if (poolData->world.hasntComps<LinearEffector, FrictionEffector>(collID)) {
				collisionFunction(collID, false, poolData->qtreeStatic, poolData->nearCollidablesBuffers[workerId], poolData->collisionInfoBuffers[workerId]);
			}
		}
		return 0; 
	}
};

inline void CollisionCheckJob::collisionFunction(
	entity_index_type collID, 
	bool otherDynamic,
	Quadtree2& qtree,
	std::vector<entity_index_type>& nearCollidablesBuffer,
	std::vector<CollisionInfo>& collisionInfos)
{
	auto& manager = poolData->world;
	auto& aabbCache = poolData->aabbCache;
	auto& collisionResponses = poolData->collisionResponses;

	if (!manager.getComp<Collider>(collID).sleeping) {
		CollidableAdapter collAdapter = CollidableAdapter(
			manager.getComp<Base>(collID).position,
			manager.getComp<Base>(collID).rotation,
			manager.getComp<Collider>(collID).size,
			manager.getComp<Collider>(collID).form,
			true,
			manager.getComp<Base>(collID).rotaVec);

		auto& baseColl = manager.getComp<Base>(collID);
		auto& colliderColl = manager.getComp<Collider>(collID);
		PosSize posSize(baseColl.position, poolData->aabbCache.at(collID));

		nearCollidablesBuffer.clear();
		qtree.querry(nearCollidablesBuffer, posSize);
		for (auto& otherID : nearCollidablesBuffer) {
			if (collID != otherID) { //do not check against self
				if (((manager.getComp<Collider>(collID).collisionMaskAgainst & manager.getComp<Collider>(otherID).collisionMaskSelf) != 0x00'00'00'00)) {
					CollidableAdapter otherAdapter = CollidableAdapter(
						manager.getComp<Base>(otherID).position,
						manager.getComp<Base>(otherID).rotation,
						manager.getComp<Collider>(otherID).size,
						manager.getComp<Collider>(otherID).form,
						otherDynamic,
						manager.getComp<Base>(otherID).rotaVec);

					auto newTestResult = collisionTestCachedAABB(collAdapter, otherAdapter, aabbCache.at(collID), aabbCache.at(otherID));
					if (newTestResult.collided) {
						if (!manager.areRelated(collID, otherID)) {
							collisionInfos.push_back(CollisionInfo(collID, otherID, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionPos));
						}
					}
				}
			}
		}
	}
}
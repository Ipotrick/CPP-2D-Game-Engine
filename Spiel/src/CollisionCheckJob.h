#pragma once
#include "JobManager.hpp"
#include "EntityComponentManager.h"
#include "QuadTree.h"
#include "collision_detection.h"
#include "Physics.h"
#include "CollisionWorker.h"

class DynCollisionCheckJob : public JobFunctor {
	std::shared_ptr<CollisionPoolData> poolData;
	std::vector<entity_index_type>& entities;
	std::vector<CollisionInfo>& collisionInfos;
	uint32_t begin = 0;
	uint32_t end = 0;
	std::vector<entity_index_type>& nearCollidablesBuffer;

	void collisionFunction(
		entity_index_type collID,
		bool otherDynamic,
		Quadtree2& qtree);
public:
	DynCollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<entity_index_type>& entities,
		std::vector<CollisionInfo>& collisionInfos,
		uint32_t begin,
		uint32_t end,
		std::vector<entity_index_type>& neighborBuffer)
		:poolData{ poolData }, entities{ entities }, collisionInfos{ collisionInfos }, begin{ begin }, end{ end }, nearCollidablesBuffer{ neighborBuffer }
	{}

	int operator()() override {
		for (uint32_t i = begin; i < end; i++) {
			auto& collID = entities.at(i);
			collisionFunction(collID, true, poolData->qtreeDynamic);
			collisionFunction(collID, false, poolData->qtreeStatic);
		}
		return 0;
	}
};

class SensorCollisionCheckJob : public JobFunctor {
	std::shared_ptr<CollisionPoolData> poolData;
	std::vector<entity_index_type>& entities;
	std::vector<CollisionInfo>& collisionInfos;
	uint32_t begin = 0;
	uint32_t end = 0;
	std::vector<entity_index_type>& nearCollidablesBuffer;

	void collisionFunction(
		entity_index_type collID,
		bool otherDynamic,
		Quadtree2& qtree);
public:
	SensorCollisionCheckJob(
		std::shared_ptr<CollisionPoolData> poolData,
		std::vector<entity_index_type>& entities,
		std::vector<CollisionInfo>& collisionInfos,
		uint32_t begin,
		uint32_t end,
		std::vector<entity_index_type>& neighborBuffer)
		:poolData{ poolData }, entities{ entities }, collisionInfos{ collisionInfos }, begin{ begin }, end{ end }, nearCollidablesBuffer{ neighborBuffer }
	{}

	int operator()() override {
		for (uint32_t i = begin; i < end; i++) {
			auto& collID = entities.at(i);
			collisionFunction(collID, true, poolData->qtreeDynamic);
			if (poolData->world.hasntComps<LinearEffector, FrictionEffector>(collID)) {
				collisionFunction(collID, false, poolData->qtreeStatic);
			}
		}
		return 0;
	}
};

inline void DynCollisionCheckJob::collisionFunction(
	entity_index_type collID, 
	bool otherDynamic,
	Quadtree2& qtree) 
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
		PosSize posSize(baseColl.position, aabbBounds(colliderColl.size, baseColl.rotaVec));

		/// dyn vs dyn
		nearCollidablesBuffer.clear();
		// querry dynamic entities
		qtree.querry(nearCollidablesBuffer, posSize);

		//check for collisions and save the changes in velocity and position these cause

		for (auto& otherID : nearCollidablesBuffer) {
			//do not check against self 
			if (collID != otherID) {
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
						if (manager.hasComp<Movement>(collID)) {
							auto& movementColl = manager.getComp<Movement>(collID);
							Vec2 velocityOther;
							if (otherDynamic) {
								auto& movementOther = manager.getComp<Movement>(otherID);
								velocityOther = movementOther.velocity;
							}
							else {
								velocityOther = Vec2(0, 0);
							}

							Vec2 newPosChange = calcPosChange(
								collAdapter.getSurfaceArea(), movementColl.velocity,
								otherAdapter.getSurfaceArea(), velocityOther,
								newTestResult.clippingDist, newTestResult.collisionNormal, otherDynamic);

							Vec2 oldPosChange = collisionResponses.at(collID).posChange;

							float weightOld = norm(oldPosChange);
							float weightNew = norm(newPosChange);
							float normalizer = weightOld + weightNew;
							if (normalizer > Physics::nullDelta) {
								collisionResponses.at(collID).posChange = (oldPosChange * weightOld / normalizer + newPosChange * weightNew / normalizer);
							}
						}
					}
				}
			}
		}
	}
}


inline void SensorCollisionCheckJob::collisionFunction(
	entity_index_type collID,
	bool otherDynamic,
	Quadtree2& qtree)
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
		PosSize posSize(baseColl.position, aabbBounds(colliderColl.size, baseColl.rotaVec));

		/// dyn vs dyn
		nearCollidablesBuffer.clear();
		// querry dynamic entities
		qtree.querry(nearCollidablesBuffer, posSize);

		//check for collisions and save the changes in velocity and position these cause

		for (auto& otherID : nearCollidablesBuffer) {
			//do not check against self 
			if (collID != otherID) {
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
						if (manager.hasComp<Movement>(collID)) {
							auto& movementColl = manager.getComp<Movement>(collID);
							Vec2 velocityOther;
							if (otherDynamic) {
								auto& movementOther = manager.getComp<Movement>(otherID);
								velocityOther = movementOther.velocity;
							}
							else {
								velocityOther = Vec2(0, 0);
							}

							Vec2 newPosChange = calcPosChange(
								collAdapter.getSurfaceArea(), movementColl.velocity,
								otherAdapter.getSurfaceArea(), velocityOther,
								newTestResult.clippingDist, newTestResult.collisionNormal, otherDynamic);

							Vec2 oldPosChange = collisionResponses.at(collID).posChange;

							float weightOld = norm(oldPosChange);
							float weightNew = norm(newPosChange);
							float normalizer = weightOld + weightNew;
							if (normalizer > Physics::nullDelta) {
								collisionResponses.at(collID).posChange = (oldPosChange * weightOld / normalizer + newPosChange * weightNew / normalizer);
							}
						}
					}
				}
			}
		}
	}
}
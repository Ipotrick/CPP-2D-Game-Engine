#pragma once

#include <thread>

#include "JobManager.hpp"
#include "PhysicsTypes.hpp"
#include "Physics.hpp"
#include "collision_detection.hpp"
#include "EntityComponentManager.hpp"

//class ImpulseResolutionJob : public JobFunctor {
//	void collisionResolution(CollisionInfo& collinfo);
//	EntityComponentManager& world;
//	robin_hood::unordered_map<uint64_t, PhysicsCollisionData>& collisionSet;
//	float deltaTime;
//	std::vector<std::vector<CollisionInfo>>& collisionBatches;
//	std::pair<int, int> batch;
//public:
//	inline size_t batchSize() { 
//		size_t size = 0;
//		for (int i = batch.first; i < batch.second; i++) {
//			size += collisionBatches[i].size();
//		}
//		return size;
//	}
//	ImpulseResolutionJob(
//		EntityComponentManager& world,
//		robin_hood::unordered_map<uint64_t, PhysicsCollisionData>& collisionSet,
//		float deltaTime,
//		std::vector<std::vector<CollisionInfo>>& collisionBatches,
//		std::pair<int, int> batch)
//		:world{ world }, collisionSet{ collisionSet }, deltaTime{ deltaTime }, collisionBatches{ collisionBatches }, batch{ batch }
//	{}
//	void execute(int workerId) override {
//		for (int i = batch.first; i < batch.second; i++) {
//			for (auto& coll : collisionBatches[i]) {
//				collisionResolution(coll);
//			}
//		}
//	}
//};
//
//inline void ImpulseResolutionJob::collisionResolution(CollisionInfo& collInfo) {
//	uint32_t entA = collInfo.indexA;
//	uint32_t entB = collInfo.indexB;
//
//	if (world.hasComp<PhysicsBody>(entA) & world.hasComp<PhysicsBody>(entB)) { //check if both are solid
//		if (!world.hasComp<Movement>(entB)) {
//			world.getComp<PhysicsBody>(entA).overlapAccum += collInfo.clippingDist * 2.0f;	// collision with wall makes greater pressure
//		}
//		else {
//			world.getComp<PhysicsBody>(entA).overlapAccum += collInfo.clippingDist;
//		}
//		applyImpulse(world, collisionSet, collInfo);
//	}
//}
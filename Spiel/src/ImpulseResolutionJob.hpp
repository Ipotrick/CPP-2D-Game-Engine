#pragma once

#include <thread>

#include "JobManager.hpp"
#include "PhysicsTypes.hpp"
#include "Physics.hpp"
#include "collision_detection.hpp"
#include "EntityComponentManager.hpp"

class ImpulseResolutionJob : public JobFunctor {
	void collisionResolution(IndexCollisionInfo& collinfo);
	EntityComponentManager& world;
	float deltaTime;
	std::vector<std::vector<IndexCollisionInfo>>& collisionBatches;
	std::pair<int, int> batch;
public:
	inline size_t batchSize() { 
		size_t size = 0;
		for (int i = batch.first; i < batch.second; i++) {
			size += collisionBatches[i].size();
		}
		return size;
	}
	ImpulseResolutionJob(
		EntityComponentManager& world,
		float deltaTime,
		std::vector<std::vector<IndexCollisionInfo>>& collisionBatches,
		std::pair<int, int> batch)
		:world{ world }, deltaTime{ deltaTime }, collisionBatches{ collisionBatches }, batch{ batch }
	{}
	int operator()(int workerId) override {
		for (int i = batch.first; i < batch.second; i++) {
			for (auto& coll : collisionBatches[i]) {
				collisionResolution(coll);
			}
		}
		return 0;
	}
};

inline void ImpulseResolutionJob::collisionResolution(IndexCollisionInfo& collInfo) {
	uint32_t entA = collInfo.indexA;
	uint32_t entB = collInfo.indexB;

	if (world.hasComp<PhysicsBody>(entA) & world.hasComp<PhysicsBody>(entB)) { //check if both are solid
		if (!world.hasComp<Movement>(entB)) {
			world.getComp<PhysicsBody>(entA).overlapAccum += collInfo.clippingDist * 2.0f;	// collision with wall makes greater pressure
		}
		else {
			world.getComp<PhysicsBody>(entA).overlapAccum += collInfo.clippingDist;
		}

		// owner stands in place for the slave for a collision response execution
		if (world.hasComp<BaseChild>(entA) | world.hasComp<BaseChild>(entB)) {
			if (world.hasComp<BaseChild>(entA) && !world.hasComp<BaseChild>(entB)) {
				entA = world.getIndex(world.getComp<BaseChild>(entA).parent);
			}
			else if (!world.hasComp<BaseChild>(entA) && world.hasComp<BaseChild>(entB)) {
				entB = world.getIndex(world.getComp<BaseChild>(entB).parent);
			}
			else {
				// both are slaves
				entA = world.getIndex(world.getComp<BaseChild>(entA).parent);
				entB = world.getIndex(world.getComp<BaseChild>(entB).parent);
			}
		}

		if (world.hasComp<PhysicsBody>(entA) & world.hasComp<PhysicsBody>(entB)) { // recheck if the owners are solid
			auto& solidA = world.getComp<PhysicsBody>(entA);
			auto& baseA = world.getComp<Base>(entA);
			auto& moveA = world.getComp<Movement>(entA);
			auto& solidB = world.getComp<PhysicsBody>(entB);
			auto& baseB = world.getComp<Base>(entB);
			Movement dummy = Movement();
			Movement& moveB = (world.hasComp<Movement>(entB) ? world.getComp<Movement>(entB) : dummy);

			auto& collidB = world.getComp<Collider>(entB);

			float elast = std::max(solidA.elasticity, solidB.elasticity);
			float friction = std::min(solidA.friction, solidB.friction) * deltaTime;
			auto [collChanges, otherChanges] = impulseResolution(
				baseA.position, moveA.velocity, moveA.angleVelocity, solidA.mass, solidA.momentOfInertia,
				baseB.position, moveB.velocity, moveB.angleVelocity, solidB.mass, solidB.momentOfInertia,
				collInfo.collisionNormal, collInfo.collisionPos, elast, friction);
			moveA.velocity += collChanges.first;
			moveA.angleVelocity += collChanges.second;
			moveB.velocity += otherChanges.first;
			moveB.angleVelocity += otherChanges.second;
		}
	}
}
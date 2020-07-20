#pragma once
#include "PhysicsTypes.h"
#include "JobManager.hpp"
#include "EntityComponentManager.h"
#include "Physics.h"
#include "collision_detection.h"

class PushoutCalcJob : public JobFunctor {
	void pushoutCalc(CollisionInfo collInfo);
	std::vector<CollisionInfo>& collisionInfos;
	std::vector<Vec2>& velocities;
	std::vector<CollisionResponse>& collisionResponses;
	EntityComponentManager& manager;
public:
	PushoutCalcJob(
		std::vector<CollisionInfo>& collisionInfos,
		std::vector<Vec2>& velocities,
		std::vector<CollisionResponse>& collisionResponses,
		EntityComponentManager& manager)
		:collisionInfos{ collisionInfos }, velocities{ velocities }, collisionResponses{ collisionResponses }, manager{ manager }
	{}

	int operator() (int workerId) override {
		for (auto& collInfo : collisionInfos) {
			pushoutCalc(collInfo);
		}
		return 0;
	}
};

void PushoutCalcJob::pushoutCalc(CollisionInfo collInfo) {
	auto& collID = collInfo.idA;
	if (manager.hasComp<Movement>(collID)) {
		auto& otherID = collInfo.idB;

		bool otherDynamic = false;
		if (manager.hasComp<Movement>(otherID)) otherDynamic = true;

		float pushoutPriority = 1.0f;
		if (Physics::pressurebasedPositionCorrection) {
			if (manager.getComp<PhysicsBody>(collID).overlapAccum > 0) {
				pushoutPriority = 2 * manager.getComp<PhysicsBody>(collID).overlapAccum / (manager.getComp<PhysicsBody>(collID).overlapAccum + manager.getComp<PhysicsBody>(otherID).overlapAccum);
			}
		}
		;
		Vec2 newPosChange = calcPosChange(
			manager.getComp<Collider>(collID).size.x * manager.getComp<Collider>(collID).size.y, velocities[collID],
			manager.getComp<Collider>(otherID).size.x * manager.getComp<Collider>(otherID).size.y, velocities[otherID],
			collInfo.clippingDist, collInfo.collisionNormal, otherDynamic, pushoutPriority);

		Vec2 oldPosChange = collisionResponses.at(collID).posChange;

		float weightOld = norm(oldPosChange);
		float weightNew = norm(newPosChange);
		float normalizer = weightOld + weightNew;
		if (normalizer > Physics::nullDelta) {
			collisionResponses.at(collID).posChange = (oldPosChange * weightOld / normalizer + newPosChange * weightNew / normalizer);
		}
	}
}
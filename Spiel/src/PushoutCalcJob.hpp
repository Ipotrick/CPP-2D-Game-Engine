#pragma once
#include "PhysicsTypes.hpp"
#include "JobManager.hpp"
#include "EntityComponentManager.hpp"
#include "Physics.hpp"
#include "collision_detection.hpp"

class PushoutCalcJob : public JobFunctor {
	std::vector<CollisionInfo>& collisionInfos;
	std::vector<Vec2>& velocities;
	std::vector<float>& overlaps;
	std::vector<CollisionResponse>& collisionResponses;
	EntityComponentManager& manager;

	void pushoutCalc(CollisionInfo collInfo);
public:
	PushoutCalcJob(
		std::vector<CollisionInfo>& collisionInfos,
		std::vector<Vec2>& velocities,
		std::vector<float>& overlaps,
		std::vector<CollisionResponse>& collisionResponses,
		EntityComponentManager& manager)
		:collisionInfos{ collisionInfos }, velocities{ velocities }, overlaps{ overlaps }, collisionResponses{ collisionResponses }, manager{ manager }
	{}

	void execute(int workerId) override {
		for (int i = 0; i < 1; i++) {
			for (auto& collInfo : collisionInfos) {
				pushoutCalc(collInfo);
			}
			for (auto& collInfo : collisionInfos) {
				auto bMoveRelativeToA = collisionResponses[collInfo.indexB].posChange - collisionResponses[collInfo.indexA].posChange;
				collInfo.clippingDist += dot((collInfo.normal[0] + collInfo.normal[1])*0.5f, bMoveRelativeToA);
			}
			for (const auto ent : manager.entity_view<Collider,PhysicsBody,Movement>()) {
				auto& base = manager.getComp<Transform>(ent);
				base.position += collisionResponses[ent].posChange * 1;
			}
		}
	}
};

inline void PushoutCalcJob::pushoutCalc(CollisionInfo collInfo) {
	auto& collID = collInfo.indexA;
	if (manager.hasComps<PhysicsBody, Movement>(collID) && manager.hasComp<PhysicsBody>(collInfo.indexB)) {
		const auto otherID = collInfo.indexB;

		bool otherDynamic = manager.hasComps<Movement>(otherID);

		const float surfaceAreaColl = manager.getComp<Collider>(collID).size.x * manager.getComp<Collider>(collID).size.y;
		const float surfaceAreaOther = manager.getComp<Collider>(otherID).size.x * manager.getComp<Collider>(otherID).size.y;
		float pushoutPriority = 1.0f;
		if (Physics::pressurebasedPositionCorrection) {
			if (Physics::pressurebasedPositionCorrection) {
				pushoutPriority = overlaps[collInfo.indexA] > overlaps[collInfo.indexB] ? 0.2f : 0.8f;
			}
		}
		Vec2 newPosChange = calcPosChange(
			surfaceAreaColl, velocities[collID],
			surfaceAreaOther, velocities[otherID],
			collInfo.clippingDist, (collInfo.normal[0] + collInfo.normal[1]) * 0.5f, otherDynamic, pushoutPriority);

		Vec2 oldPosChange = collisionResponses.at(collID).posChange;

		float weightOld = norm(oldPosChange);
		float weightNew = norm(newPosChange);
		float normalizer = weightOld + weightNew;
		if (normalizer > Physics::nullDelta) {
			collisionResponses.at(collID).posChange = (oldPosChange * weightOld / normalizer + newPosChange * weightNew / normalizer);
		}
	}
}

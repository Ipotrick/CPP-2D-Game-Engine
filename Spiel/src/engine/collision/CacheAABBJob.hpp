#pragma once

#include "CollisionUniform.hpp"
#include "../../engine/JobSystem.hpp"
#include "../../engine/entity/EntityComponentStorage.hpp"
#include "../../engine/entity/EntityComponentManagerView.hpp"
#include "../../engine/math/Vec2.hpp"

class CacheAABBJob : public IJob {
public:
	CacheAABBJob(std::vector<EntityHandleIndex>& entities_to_cache, CollisionSECM manager, std::vector<Vec2>& aabbs)
		:entities_to_cache{ entities_to_cache }, manager{ manager }, aabbs{ aabbs } 
	{}
	void execute(const uint32_t workerId) override {
		for (auto ent : entities_to_cache) {
			const Transform& base = manager.getComp<Transform>(ent);
			const Collider& collider = manager.getComp<Collider>(ent);
			aabbs.at(ent) = collider.form == Form::Circle ? collider.size : aabbBounds(collider.size, base.rotaVec);
			for (auto& c : collider.extraColliders) {
				Vec2 aabb = c.form == Form::Circle ? c.size : aabbBounds(c.size, base.rotaVec * c.relativeRota);
				Vec2 offset = rotate(c.relativePos, base.rotaVec);
				aabb += abs(offset)*2;
				aabbs.at(ent) = max(aabbs.at(ent), aabb);
			}
		}
	}
private:
	std::vector<EntityHandleIndex>& entities_to_cache;
	CollisionSECM manager;
	std::vector<Vec2>& aabbs;
};
#pragma once

#include "JobManager.hpp"
#include "EntityComponentStorage.hpp"
#include "EntityComponentManager.hpp"
#include "Vec2.hpp"

class CacheAABBJob : public JobFunctor{
public:
	CacheAABBJob(std::vector<EntityHandleIndex>& entities_to_cache, EntityComponentManager& manager, std::vector<Vec2>& aabbs)
		:entities_to_cache{ entities_to_cache }, manager{ manager }, aabbs{ aabbs } 
	{}
	void execute(int workerId) override {
		for (auto ent : entities_to_cache) {
			auto base = manager.getComp<Transform>(ent);
			auto collider = manager.getComp<Collider>(ent);
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
	EntityComponentManager& manager;
	std::vector<Vec2>& aabbs;
};
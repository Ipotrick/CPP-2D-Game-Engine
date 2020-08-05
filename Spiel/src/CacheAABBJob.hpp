#pragma once

#include "JobManager.hpp"
#include "EntityComponentStorage.hpp"
#include "EntityComponentManager.hpp"
#include "Vec2.hpp"

class CacheAABBJob : public JobFunctor{
	std::vector<Entity>& entities_to_cache;
	EntityComponentManager& manager;
	std::vector<Vec2>& aabbs;
public:
	CacheAABBJob(std::vector<Entity>& entities_to_cache, EntityComponentManager& manager, std::vector<Vec2>& aabbs)
		:entities_to_cache{ entities_to_cache }, manager{ manager }, aabbs{ aabbs } 
	{}
	void execute(int workerId) override {
		for (auto ent : entities_to_cache) {
			auto& base = manager.getComp<Base>(ent);
			auto& collider = manager.getComp<Collider>(ent);
			if (collider.form == Form::Circle) {
				aabbs.at(ent) = collider.size;
			}
			else {
				aabbs.at(ent) = aabbBounds(collider.size, base.rotaVec);
			}
		}
	}
};
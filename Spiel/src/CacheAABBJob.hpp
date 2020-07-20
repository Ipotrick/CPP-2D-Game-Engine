#pragma once

#include "JobManager.hpp"
#include "EntityComponentStorage.h"
#include "EntityComponentManager.h"
#include "Vec2.h"

class CacheAABBJob : public JobFunctor{
	std::vector<entity_index_type>& entities_to_cache;
	EntityComponentManager& manager;
	std::vector<Vec2>& aabbs;
public:
	CacheAABBJob(std::vector<entity_index_type>& entities_to_cache, EntityComponentManager& manager, std::vector<Vec2>& aabbs)
		:entities_to_cache{ entities_to_cache }, manager{ manager }, aabbs{ aabbs } 
	{}
	int operator()(int workerId) override {
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
		return 0;
	}
};
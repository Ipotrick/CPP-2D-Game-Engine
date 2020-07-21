#pragma once

#include "JobManager.hpp"
#include "EntityComponentManager.hpp"
#include "QuadTree.hpp"

class BuildQtreeJob : public JobFunctor {
	EntityComponentManager& manager;
	std::vector<entity_id_type>& entities;
	Quadtree2& qtree;
public:
	BuildQtreeJob(EntityComponentManager& manager, std::vector<entity_id_type>& entities, Quadtree2& qtree)
		: manager{ manager }, entities{ entities }, qtree{ qtree }
	{}
	int operator()(int workerId) override {
		for (auto& ent : entities) {
			if (!manager.getComp<Collider>(ent).particle) {	// never check for collisions against particles
				qtree.insert(ent);
			}
		}
		return 0;
	}
};
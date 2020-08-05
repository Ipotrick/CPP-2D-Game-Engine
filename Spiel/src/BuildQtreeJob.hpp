#pragma once

#include "JobManager.hpp"
#include "EntityComponentManager.hpp"
#include "QuadTree.hpp"

class BuildQtreeJob : public JobFunctor {
	EntityComponentManager& manager;
	std::vector<Entity>& entities;
	Quadtree2& qtree;
	bool insertParticles;
	std::vector<Vec2>& aabbCache;
public:
	BuildQtreeJob(EntityComponentManager& manager, std::vector<entity_id_t>& entities, Quadtree2& qtree, std::vector<Vec2>& aabbCache, bool insertParticles)
		: manager{ manager }, entities{ entities }, qtree{ qtree }, insertParticles{ insertParticles }, aabbCache{ aabbCache }
	{}
	void execute(int workerId) override {
		for (const auto ent : entities) {
			if (insertParticles) {
				if (manager.getComp<Collider>(ent).particle) {
					qtree.insert(ent, aabbCache.at(ent));
				}
			}
			else {
				if (!manager.getComp<Collider>(ent).particle) {	// never check for collisions against particles
					qtree.insert(ent, aabbCache.at(ent));
				}
			}
		}
	}
};
#pragma once

#include "Physics.h"
#include "QuadTree.h"

struct sharedPhysicsData {

};

class PhysicsWorker {
public:
	PhysicsWorker(std::vector<Collidable*> const& dynCollidables_, int begin_, int end_, Quadtree const& qtree_, std::vector<CollisionResponse>& collisionResponses_, std::vector<CollisionInfo>& collisionInfos_) :
		dynCollidables{ dynCollidables_ },
		begin{ begin_ },
		end{ end_ },
		qtree{ qtree_ },
		collisionResponses{ collisionResponses_ },
		collisionInfos{ collisionInfos_ }
	{

	}

	std::vector<Collidable*> const& dynCollidables;
	int begin;
	int end;
	Quadtree const& qtree;
	std::vector<CollisionResponse>& collisionResponses;
	std::vector<CollisionInfo>& collisionInfos;

	void operator()(); 
};
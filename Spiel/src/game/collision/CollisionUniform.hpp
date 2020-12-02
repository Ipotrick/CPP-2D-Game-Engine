#pragma once

#include "../../engine/EngineCore.hpp"
#include "CoreComponents.hpp"

#define COLLISION_SECM_COMPONENTS \
	Transform,\
	Movement, \
	Collider,\
	PhysicsBody, \
	CollisionsToken

using CollisionSECM = SubEntityComponentManager<
	ComponentStoragePagedIndexing<Transform>,
	ComponentStoragePagedIndexing<Movement>,
	ComponentStoragePagedIndexing<Collider>,
	ComponentStoragePagedIndexing<PhysicsBody>,
	ComponentStoragePagedIndexing<CollisionsToken>
>;
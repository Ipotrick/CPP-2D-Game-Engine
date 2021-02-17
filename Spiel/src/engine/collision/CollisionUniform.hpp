#pragma once

#include "../../engine/EngineCore.hpp"
#include "CoreComponents.hpp"

#define COLLISION_SECM_COMPONENTS \
	Transform,\
	Movement, \
	Collider,\
	PhysicsBody, \
	CollisionsToken

using CollisionSECM = EntityComponentManagerView <
	ComponentStoragePagedIndexing<Transform>,
	ComponentStoragePagedIndexing<Movement>,
	ComponentStoragePagedIndexing<Collider>,
	ComponentStoragePagedIndexing<PhysicsBody>,
	ComponentStoragePagedIndexing<CollisionsToken>
>;
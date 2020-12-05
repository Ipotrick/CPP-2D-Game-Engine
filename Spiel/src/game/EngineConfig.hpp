#pragma once

#include <type_traits>

#include "collision/CoreComponents.hpp"
#include "GameComponents.hpp"
#include "../engine/entity/EntityComponentStorage.hpp"
#include "../engine/types/BaseTypes.hpp"
#include "../engine/rendering/RenderTypes.hpp"

#define ENGINE_COMPONENT_LIST \
	ComponentStoragePagedIndexing<Transform>,\
	ComponentStoragePagedIndexing<Draw>,\
	ComponentStoragePagedIndexing<Collider>,\
	ComponentStoragePagedIndexing<CollisionsToken>,\
	ComponentStoragePagedIndexing<Movement>,\
	ComponentStoragePagedIndexing<PhysicsBody>,\
	ComponentStoragePagedSet<TextureRef2>,\
	ComponentStoragePagedSet<LinearEffector>,\
	ComponentStoragePagedSet<FrictionEffector>,\
	ComponentStoragePagedSet<Health>,\
	ComponentStoragePagedSet<Age>,\
	ComponentStoragePagedSet<Player>,\
	ComponentStoragePagedSet<Bullet>,\
	ComponentStoragePagedSet<Enemy>,\
	ComponentStoragePagedSet<ParticleScriptComp>,\
	ComponentStoragePagedSet<Dummy>,\
	ComponentStoragePagedSet<SpawnerComp>,\
	ComponentStoragePagedSet<SuckerComp>,\
	ComponentStoragePagedSet<Tester>
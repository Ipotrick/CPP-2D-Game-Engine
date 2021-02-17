#pragma once

#include <type_traits>

#include "GameComponents.hpp"
#include "../engine/collision/CoreComponents.hpp"
#include "../engine/entity/EntityComponentStorage.hpp"
#include "../engine/types/BaseTypes.hpp"
#include "../engine/rendering/OpenGLAbstraction/OpenGLTexture.hpp"

#define ENGINE_COMPONENT_LIST \
	ComponentStoragePagedIndexing<Transform>,\
	ComponentStoragePagedIndexing<Draw>,\
	ComponentStoragePagedIndexing<Collider>,\
	ComponentStoragePagedIndexing<CollisionsToken>,\
	ComponentStoragePagedIndexing<Movement>,\
	ComponentStoragePagedIndexing<PhysicsBody>,\
	ComponentStoragePagedSet<TextureDescriptor>,\
	ComponentStoragePagedSet<TextureSection>,\
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
#include "CoreComponents.hpp"
#include "GameComponents.hpp"
#include "EntityComponentStorage.hpp"
#include "BaseTypes.hpp"
#include "Renderer.hpp"

using ComponentStorageTuple = 
std::tuple<
	// Core Components:
	ComponentStorage<Transform, paged_indexing>,
	ComponentStorage<Draw, paged_indexing>,
	ComponentStorage<Collider, paged_indexing>,
	ComponentStorage<CollisionsToken, paged_indexing>,
	ComponentStorage<Movement, paged_indexing>,
	ComponentStorage<PhysicsBody, paged_indexing>,
	ComponentStorage<TextureRef2, paged_set>,
	ComponentStorage<LinearEffector, paged_set>,
	ComponentStorage<FrictionEffector, paged_set>,
	// Game Components:
	ComponentStorage<Health, paged_set>,
	ComponentStorage<Age, paged_set>,
	ComponentStorage<Player, paged_set>,
	ComponentStorage<Bullet, paged_set>,
	ComponentStorage<Enemy, paged_set>,
	ComponentStorage<ParticleScriptComp, paged_set>,
	ComponentStorage<Dummy, paged_set>,
	ComponentStorage<SpawnerComp, paged_set>,
	ComponentStorage<SuckerComp, paged_set>,
	ComponentStorage<Tester, paged_set>
>;
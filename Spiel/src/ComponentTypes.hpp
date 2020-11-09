#include <type_traits>

#include "CoreComponents.hpp"
#include "GameComponents.hpp"
#include "EntityComponentStorage.hpp"
#include "BaseTypes.hpp"
#include "Renderer.hpp"


/*
* This tuple defines wich types the EntityComponentManager can store and how they are stored.
* if this tuple is changed in any way, the whole solution and the engine MUST be recomlpiled!
*/
using ComponentStorageTuple =
std::tuple <
	// Core Components:
	ComponentStoragePagedIndexing<Transform>,
	ComponentStoragePagedIndexing<Draw>,
	ComponentStoragePagedIndexing<Collider>,
	ComponentStoragePagedIndexing<CollisionsToken>,
	ComponentStoragePagedIndexing<Movement>,
	ComponentStoragePagedIndexing<PhysicsBody>,
	ComponentStoragePagedSet<TextureRef2>,
	ComponentStoragePagedSet<LinearEffector>,
	ComponentStoragePagedSet<FrictionEffector>,
	// Game Components:
	ComponentStoragePagedSet<Health>,
	ComponentStoragePagedSet<Age>,
	ComponentStoragePagedSet<Player>,
	ComponentStoragePagedSet<Bullet>,
	ComponentStoragePagedSet<Enemy>,
	ComponentStoragePagedSet<ParticleScriptComp>,
	ComponentStoragePagedSet<Dummy>,
	ComponentStoragePagedSet<SpawnerComp>,
	ComponentStoragePagedSet<SuckerComp>,
	ComponentStoragePagedSet<Tester>
> ;



template< size_t I, typename T>
static constexpr size_t findIndexInComponentStorageTuple()
{
	static_assert(I < std::tuple_size<ComponentStorageTuple>::value, "the given component type is unknown");

	using el = typename std::tuple_element<I, ComponentStorageTuple>::type;
	if constexpr (
		std::is_same<ComponentStorageDirectIndexing<T>, el>::value
		|| std::is_same<ComponentStoragePagedIndexing<T>, el>::value
		|| std::is_same<ComponentStoragePagedSet<T>, el>::value
		) {
		return I;
	}
	else {
		return findIndexInComponentStorageTuple<I + 1, T>();
	}
}
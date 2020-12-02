#pragma once
#include <type_traits>

#include "EntityComponentStorage.hpp"
#include "BaseTypes.hpp"
#include "Renderer.hpp"
//#include "../EngineConfig.hpp"

/**
 * Define your own components in the makro: ENGINE_COMPONENT_LIST.
 * define the makro beforehand every time you include the Engine.hpp file.
 * The syntax is like in the makro: ENGINE_CORE_COMPONENT_LIST.
 * There are 3 possible Storage containers for your types:
 *		1. ComponentStorageDirectIndexing		// best random access, bad sequential access, very bad memory consumtion
 *		2. ComponentStoragePagedIndexing		// good random access, good sequential access, very good memory consumtion for small types, bad for large types
 *		3. ComponentStoragePagedSet				// good random access, best sequential access, best memory consumtion
 */

#ifndef ENGINE_COMPONENT_LIST
static_assert(false);
#endif

/*
* This tuple defines wich types the EntityComponentManager can store and how they are stored.
* if this tuple is changed in any way, the whole solution and the engine MUST be recomlpiled!
*/
#ifndef ENGINE_COMPONENT_LIST
using ComponentStorageTuple = std::tuple<int>;
#else
using ComponentStorageTuple = std::tuple<ENGINE_COMPONENT_LIST>;
#endif

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
#pragma once

using EntityHandleIndex = uint32_t;
using EntityHandleVersion = uint16_t;

static constexpr EntityHandleIndex INVALID_ENTITY_HANDLE_INDEX{ 0xFFFFFFFF };
static constexpr EntityHandleIndex INVALID_ENTITY_HANDLE_VERSION{ 0xFFFF };

/*
* An EntityHandle...
*	is used to refer to an entity in one specific world.
*	can be used to refer to the entity over multiple frames.
*	is invalidated when the entity is serialised and deserialised.
*	is invalidated when the entity is saved and loaded from another world.
*/
struct EntityHandle {
	EntityHandle() = default; 
	EntityHandle(EntityHandleIndex index, EntityHandleVersion version)
		:index{ index }, version{ version }
	{}
	EntityHandleIndex index{ INVALID_ENTITY_HANDLE_INDEX };
	EntityHandleVersion version{ INVALID_ENTITY_HANDLE_VERSION };

	bool operator==(const EntityHandle& other) const { return (this->index == other.index) & (this->version == other.version); }
	bool operator!=(const EntityHandle& other) const { return (this->index != other.index) | (this->version != other.version); }
};
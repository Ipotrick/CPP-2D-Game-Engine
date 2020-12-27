#pragma once

using EntityHandleIndex = uint32_t;
using EntityHandleVersion = uint16_t;

static constexpr EntityHandleIndex INVALID_ENTITY_HANDLE_INDEX{ 0xFFFFFFFF };
static constexpr EntityHandleIndex INVALID_ENTITY_HANDLE_VERSION{ 0xFFFF };

/*
* An EntityHandle...
*	is used to refer to an entity in one specific world.
*   is a unique identifier in the entity manager it was created in for the duration of the entity managers existence.
*	can be used to refer to the entity over multiple frames.
*	is invalidated when the entity is serialised and deserialised.
*	is invalidated when the entity is saved and loaded from another entity manager.
*/
struct EntityHandle {
	EntityHandle() = default; 
	EntityHandle(EntityHandleIndex index, EntityHandleVersion version)
		:index{ index }, version{ version }
	{}
	EntityHandleIndex index{ INVALID_ENTITY_HANDLE_INDEX };
	EntityHandleVersion version{ INVALID_ENTITY_HANDLE_VERSION };

	bool valid() const { return (this->index != INVALID_ENTITY_HANDLE_INDEX) & (this->version != INVALID_ENTITY_HANDLE_VERSION); }

	bool operator==(const EntityHandle& other) const { return (this->index == other.index) & (this->version == other.version); }
	bool operator!=(const EntityHandle& other) const { return (this->index != other.index) | (this->version != other.version); }
};
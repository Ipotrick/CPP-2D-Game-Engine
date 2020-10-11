#pragma once
#include <boost/serialization/access.hpp>

using entity_id_t = uint32_t;
using Entity = uint32_t;
static constexpr uint32_t INVALID_ID{ 0xFFFFFFFF };
static constexpr Entity INVALID_ENTITY{ 0xFFFFFFFF };
struct EntityId {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& identifier;
		ar& version;
	}
	explicit EntityId(entity_id_t id = 0, uint32_t version = 0) : identifier{ id }, version{ version } {}
	void operator=(EntityId rhs) {
		this->identifier = rhs.identifier;
		this->version = rhs.version;
	}
	bool operator==(EntityId const other)
	{
		return this->identifier == other.identifier && this->version == other.version;
	}
	entity_id_t& operator*() { return identifier; }
	entity_id_t identifier;
	uint32_t version;
};
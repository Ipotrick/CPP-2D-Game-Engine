#pragma once
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using entity_id_t = uint32_t;
using Entity = uint32_t;
struct EntityId {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& id;
		ar& version;
	}
	explicit EntityId(entity_id_t id = 0, uint32_t version = 0) : id{ id }, version{ version } {}
	void operator=(EntityId rhs) {
		this->id = rhs.id;
		this->version = rhs.version;
	}
	bool operator==(EntityId const other)
	{
		return this->id == other.id && this->version == other.version;
	}
	entity_id_t& operator*() { return id; }
	entity_id_t id;
	uint32_t version;
};
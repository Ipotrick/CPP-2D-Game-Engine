#pragma once

using entity_id_type = uint32_t;
using entity_index_type = uint32_t;
struct entity_id {
	explicit entity_id(entity_id_type id = 0, uint32_t version = 0) : id{ id }, version{ version } {}
	void operator=(entity_id rhs) {
		this->id = rhs.id;
		this->version = rhs.version;
	}
	entity_id_type& operator*() { return id; }
	entity_id_type id;
	uint32_t version;
};
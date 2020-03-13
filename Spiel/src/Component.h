#pragma once
#pragma once

#include "robin_hood.h"

struct CompData {};

template<typename CompDataType>
struct CompController {
	friend class World;
	CompController() : componentData{}
	{}

	/* register an entitiy to be affected by the component. The parameter is the constructor parameter for the component data, O(1) !MAY CAUSE A REALLOCATION! */
	void registerEntity(uint32_t id, CompDataType const& newComponentData);
	/* remove entitiy from the infruence of the component, O(n) */
	void deregisterEntity(uint32_t id);
	/* returns true if an entity with the given id is registered in the ComponentController, O(log2(n)) */
	bool isRegistered(uint32_t);
	/* returns a pointer to the componentData of the given entity. If the given id is not registered nullptr will be returned, O(log2(n)) */
	CompDataType * const getComponent(uint32_t id);
	robin_hood::unordered_map<uint32_t, CompDataType> componentData;
};

template<typename CompDataType>
inline void CompController<CompDataType>::registerEntity(uint32_t id, CompDataType const& newComponentData)
{
	componentData.insert({ id, newComponentData });
}

template<typename ComponentDataType>
void CompController<ComponentDataType>::deregisterEntity(uint32_t id)
{
	auto res = componentData.find(id);
	if (res != componentData.end() && res->first == id) {
		componentData.erase(res);
	}
}

template<typename CompDataType>
inline bool CompController<CompDataType>::isRegistered(uint32_t id)
{
	auto res = componentData.find(id);
	if (res != componentData.end()) {
		if (res->first == id) {
			return true;
		}
	}
	return false;
}

template<typename CompDataType>
inline CompDataType * const CompController<CompDataType>::getComponent(uint32_t id)
{
	auto res = componentData.find(id);
	if (res != componentData.end() && res->first == id) {
		return &res->second;
	}
	else {
		return nullptr;
	}
}
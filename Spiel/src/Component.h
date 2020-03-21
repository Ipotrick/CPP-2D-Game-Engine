#pragma once
#pragma once

#include <assert.h>

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
	bool isRegistered(uint32_t id);
	/* returns a pointer to the componentData of the given entity. If the given id is not registered nullptr will be returned, O(1) (hash access) */
	CompDataType * const getComponentPtr(uint32_t id);
	/* returns a reference to the componentData of a given entity. UNCHECKED! CALL isRegistered BEFORE USE!, O(1) */
	CompDataType& getComponent(uint32_t id);
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
	if (res != componentData.end() && res->first == id) {
		return true;
	}
	return false;
}

template<typename CompDataType>
inline CompDataType * const CompController<CompDataType>::getComponentPtr(uint32_t id)
{
	auto res = componentData.find(id);
	if (res != componentData.end()) {
		return &res->second;
	}
	else {
		return nullptr;
	}
}

template<typename CompDataType>
CompDataType& CompController<CompDataType>::getComponent(uint32_t id) 
{
	assert(isRegistered(id));
	return componentData[id];
}

template<typename CompDataType>
struct CompControllerLUT {
	friend class World;

	/* register an entitiy to be affected by the component. The parameter is the constructor parameter for the component data, O(1) !MAY CAUSE A REALLOCATION! */
	void registerEntity(uint32_t id, CompDataType const& newComponentData);
	/* remove entitiy from the infruence of the component, O(n) */
	void deregisterEntity(uint32_t id);
	/* returns true if an entity with the given id is registered in the ComponentController, O(log2(n)) */
	bool isRegistered(uint32_t);
	/* returns a pointer to the componentData of the given entity. If the given id is not registered nullptr will be returned, O(1) */
	CompDataType* const getComponentPtr(uint32_t id);
	/* returns a reference to the componentData of a given entity. UNCHECKED! CALL isRegistered BEFORE USE!, O(1) */
	CompDataType& getComponent(uint32_t id);
	std::vector<std::pair<bool, CompDataType>> componentData;
};

template<typename CompDataType>
inline void CompControllerLUT<CompDataType>::registerEntity(uint32_t id, CompDataType const& newComponentData)
{
	if (id < componentData.size()) {
		assert(componentData[id].first != true);	//cant register a registered entity
		componentData[id].first = true;
		componentData[id].second = newComponentData;
	}
	else if (id == componentData.size()) {
		componentData.push_back({ true, newComponentData });
	}
	else if (id > componentData.size()) {
		componentData.resize(id, std::pair<bool, CompDataType>(false, CompDataType()));
		componentData.push_back({ true, newComponentData });
	}
}

template<typename CompDataType>
inline void CompControllerLUT<CompDataType>::deregisterEntity(uint32_t id)
{
	assert(id < componentData.size()); //cannot deregister a component that doesnt even have storage space
	componentData[id].first = false;
}

template<typename CompDataType>
inline bool CompControllerLUT<CompDataType>::isRegistered(uint32_t id)
{
	if (id < componentData.size()) {
		return componentData[id].first;
	}
	else {
		return false;
	}
}

template<typename CompDataType>
inline CompDataType* const CompControllerLUT<CompDataType>::getComponentPtr(uint32_t id)
{
	if (isRegistered(id)) {
		return &componentData[id].second;
	}
	else {
		return nullptr;
	}
}

template<typename CompDataType>
CompDataType& CompControllerLUT<CompDataType>::getComponent(uint32_t id)
{
	assert(isRegistered(id));
	return componentData[id].second;
}
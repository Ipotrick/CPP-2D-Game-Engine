#pragma once

#include <vector>

#include "Entity.h"

struct CompData {
	CompData(uint32_t id_) : id{id_} {}
	uint32_t id;
};

// basic ComponentController

template<typename CompDataType>
class BasicCompController {
public:
	BasicCompController(): componentData{}
	{}

	/* register an entitiy to be affected by the component. The parameter is the constructor parameter for the component data, O(1) !MAY CAUSE A REALLOCATION! */
	void registerEntity(CompDataType const& componentData);
	/* remove entitiy from the infruence of the component, O(n) */
	void deregisterEntity(uint32_t id);
	/* returns true if an entity with the given id is registered in the ComponentController, O(log2(n)) */
	bool isRegistered(uint32_t);
	/* returns a pointer to the componentData of the given entity. If the given id is not registered nullptr will be returned, O(log2(n)) */
	CompDataType const* const getComponent(uint32_t id);
	/* executes the component script on all registered componentes, O(n)*/
	void executeScripts(float deltaTime);

protected:
	/* script for one component, O(1) */
	virtual void executeScript(CompDataType& data, float deltaTime) = 0;

protected:
	std::vector<CompDataType> componentData;
};

template<typename CompDataType>
inline void BasicCompController<CompDataType>::registerEntity(CompDataType const& newComponentData)
{
	componentData.push_back(newComponentData);
}

template<typename ComponentDataType>
void BasicCompController<ComponentDataType>::deregisterEntity(uint32_t id)
{
	for (auto iter = componentData.begin(); iter != componentData.end(); ++iter) {
		if (iter->id == id) {
			componentData.erase(iter);
			break;
		}
	}
}

template<typename CompDataType>
inline bool BasicCompController<CompDataType>::isRegistered(uint32_t id)
{
	auto iter = std::lower_bound(componentData.begin(), componentData.end(), id,
		[](CompDataType const& a, uint32_t b)
		{
			return a.id < b;
		}
	);
	if (iter != componentData.end() && iter->id == id) {
		return true;
	}
	else {
		return false;
	}
}

template<typename CompDataType>
inline CompDataType const* const BasicCompController<CompDataType>::getComponent(uint32_t id)
{
	auto iter = std::lower_bound(componentData.begin(), componentData.end(), id,
		[](CompDataType const& a, uint32_t b)
		{
			return a.id < b;
		}
	);
	if (iter != componentData.end() && iter->id == id) {
		return &*iter;
	}
	else {
		return nullptr;
	}
}

template<typename CompDataType>
inline void BasicCompController<CompDataType>::executeScripts(float deltaTime)
{
	for (auto& el : componentData) {
		executeScript(el, deltaTime);
	}
}
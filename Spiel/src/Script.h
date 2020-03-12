#pragma once

#include "Component.h"
#include "World.h"

template<typename CompDataType, class CompController>
class ScriptController {
public:
	ScriptController(CompController& cmpCtrl) : CompController{ cmpCtrl } {}

	void executeScripts(World& world, float deltaTime);
	virtual executeScript(uint32_t id, CompDataType& data, World& world, float deltaTime) = 0;
private:
	CompController& compController;
};

template<typename CompDataType, class CompController>
inline void ScriptController<CompDataType, CompController>::executeScripts(World& world, float deltaTime) {
	for (auto& comp : compController.componentData) {
		executeScript(comp.first, comp.second, world, deltaTime);
	}
}
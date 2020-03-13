#pragma once

#include "Component.h"
#include "World.h"

template<typename CompDataType, class CompController>
class ScriptController {
public:
	ScriptController(CompController& cmpCtrl_, Engine& engine_) : compController{ cmpCtrl_ }, engine{ engine_ } {}

	/* calls "executeSample()" on every component in of the componentController that is given in constructor */
	void executeAll(World& world, float deltaTime);
	/* a custom script one has to set up, that prefferably only changes the sample component */
	virtual void executeSample(uint32_t id, CompDataType& data, World& world, float deltaTime) = 0;
	/* a custom preperation script that is called in executeAll before calling exeuteSample on evey comp. Can be left empty if there is no need to prepare anything */
	virtual void executeMeta(World& world, float deltaTime) = 0;
protected:
	CompController& compController;
	Engine& engine;
};

template<typename CompDataType, class CompController>
inline void ScriptController<CompDataType, CompController>::executeAll(World& world, float deltaTime) {
	executeMeta(world, deltaTime);
	for (auto& comp : compController.componentData) {
		executeSample(comp.first, comp.second, world, deltaTime);
	}
}
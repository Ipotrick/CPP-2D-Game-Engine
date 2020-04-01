#pragma once

#include "ECS.h"
#include "World.h"

template<typename CompDataType>
class ScriptController {
public:
	ScriptController(Engine& engine_) : engine{ engine_ } {}

	/* calls "executeSample" on every component in of the componentController that is given in constructor */
	void executeAll(World& world, float deltaTime);
	/* a custom script one has to set up, that prefferably only changes the sample component */
	virtual void executeSample(uint32_t id, CompDataType& data, World& world, float deltaTime) = 0;
	/* a custom preperation script that is called in executeAll before calling exeuteSample on evey comp. Can be left empty if there is no need to prepare anything */
	virtual void executeMeta(World& world, float deltaTime) = 0;
protected:
	Engine& engine;
};

template<typename CompDataType>
inline void ScriptController<CompDataType>::executeAll(World& world, float deltaTime) {
	executeMeta(world, deltaTime);
	auto& view = world.view<CompDataType>();
	for (auto iter = view.begin(); iter != view.end(); ++iter) {
		executeSample(iter.id(), *iter, world, deltaTime);
	}
}
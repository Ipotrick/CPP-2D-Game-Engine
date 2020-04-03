#pragma once
#include "Engine.h"

template<typename CompType>
class ScriptController {
public:
	ScriptController(Engine& engine_) : engine{ engine_ } {}

	/* calls "executeSample" on every component in of the componentController that is given in constructor */
	void executeAll(float deltaTime);
	/* a custom script one has to set up, that prefferably only changes the sample component */
	virtual void executeSample(uint32_t id, CompType& data, float deltaTime) = 0;
	/* a custom preperation script that is called in executeAll before calling exeuteSample on evey comp. Can be left empty if there is no need to prepare anything */
	virtual void executeMeta(float deltaTime) = 0;
protected:
	Engine& engine;
	std::vector<std::pair<ent_id_t, CompType>> delayedAddComp;
};

template<typename CompType>
inline void ScriptController<CompType>::executeAll(float deltaTime) {
	executeMeta( deltaTime);

	for (auto entity : engine.world.view<CompType>()) {
		executeSample(entity, engine.world.getComp<CompType>(entity), deltaTime);
	}

	for (auto& comp : delayedAddComp) {
		engine.world.addComp<CompType>(comp.first, comp.second);
	}
	delayedAddComp.clear();
}
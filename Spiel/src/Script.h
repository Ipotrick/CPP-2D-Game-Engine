#pragma once
#include <functional>
#include "Engine.h"

#define GAME_SCRIPT(name, Component) \
struct name : public GameScript<Component> { \
using GameScript<Component>::GameScript; \
virtual void script(entity_handle id, Component& data, float deltaTime) override; \
};

template<typename CompType>
class GameScript {
public:
	GameScript(Engine& engine) : engine{ engine }, world{ engine.world } { }

	/* calls "executeSample" on every component in of the componentController that is given in constructor */
	void execute(float deltaTime);
	virtual void script(entity_handle, CompType&, float) = 0;
protected:
	Engine& engine;
	World& world;
};

template<typename CompType>
inline void GameScript<CompType>::execute(float deltaTime) {

	for (auto entity : engine.world.view<CompType>()) {
		script(entity, engine.world.getComp<CompType>(entity), deltaTime);
	}
}
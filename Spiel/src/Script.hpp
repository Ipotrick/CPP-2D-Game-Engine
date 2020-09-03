#pragma once
#include <functional>
#include "Engine.hpp"

/*
	name is the name of the class that holds the script
	Component is the Component datatype that the script is attached to
*/
#define GAME_SCRIPT(name, Component) \
struct name : public GameScript<Component> { \
using GameScript<Component>::GameScript; \
virtual void script(Entity id, Component& data, float deltaTime) override; \
};

template<typename CompType>
class GameScript {
public:
	GameScript(Engine& engine) : engine{ engine }, world{ engine.world } { }

	/* calls "executeSample" on every component in of the componentController that is given in constructor */
	void execute(float deltaTime);
	virtual void script(Entity, CompType&, float) = 0;
protected:
	Engine& engine;
	World& world;
};

template<typename CompType>
inline void GameScript<CompType>::execute(float deltaTime) {

	for (auto entity : engine.world.entity_view<CompType>()) {
		script(entity, engine.world.getComp<CompType>(entity), deltaTime);
	}
}
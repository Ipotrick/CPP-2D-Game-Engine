#include "TesterScript.hpp"

void TesterScript::script(Entity me, Tester& data, float deltaTime)
{
	if (world.hasComps<Base, Draw>(me)) {
		auto collView = engine.collisionSystem.collisions_view(me);
		bool foundPlayer = false;
		for (auto c : collView) {
			if (world.hasComp<Player>(c.indexB)) {
				foundPlayer = true;
				break;
			}
		}

		if (world.hasComp<Draw>(me)) {
			world.getComp<Draw>(me).color = foundPlayer ? Vec4(1,1,1,0.5f) : Vec4(0,0,0,0);
		}
	}
}
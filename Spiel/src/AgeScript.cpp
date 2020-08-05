#include "AgeScript.hpp"

void AgeScript::script(Entity id, Age& data, float deltaTime) {

	data.curAge += deltaTime;

	if (data.curAge > data.maxAge) {
		engine.world.destroy(id);
	}
}
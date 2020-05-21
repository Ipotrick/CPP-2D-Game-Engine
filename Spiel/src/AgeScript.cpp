#include "AgeScript.h"

void AgeScript::script(entity_id id, Age& data, float deltaTime) {

	data.curAge += deltaTime;

	if (data.curAge > data.maxAge) {
		engine.world.destroy(id);
	}
}
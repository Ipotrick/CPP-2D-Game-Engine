#include "TesterScript.hpp"

void TesterScript::script(Entity me, Tester& data, float deltaTime)
{
	if (world.hasComps<Base, Draw>(me)) {
		Base& b = engine.world.getComp<Base>(me);
		Draw& d = engine.world.getComp<Draw>(me);
		data.changeDirTime += deltaTime;
		if (data.changeDirTime > 1.0f)
			data.changeDirTime = -1.0f;

		b.rotation = 360 * data.changeDirTime;
		b.position += Vec2(0, 1) * data.changeDirTime;
		d.color += Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f);
	}
}
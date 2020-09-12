#include "TesterScript.hpp"

void TesterScript::script(Entity me, Tester& data, float deltaTime)
{
	if (world.hasComps<Base, Draw>(me)) {
		auto [b, d] = engine.world.getComps<Base, Draw>(me);
		data.changeDirTime += deltaTime;
		if (data.changeDirTime > 1.0f)
			data.changeDirTime = -1.0f;

		b.rotation = 360 * data.changeDirTime;
		b.position += Vec2(0, 0.1) * data.changeDirTime;
		d.color += Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f);
	}
}
#include "TesterScript.hpp"

void TesterScript::script(EntityHandle me, Tester& data, float deltaTime)
{
	if (world.hasComps<Transform, Draw>(me)) {
		auto [b, d] = engine.world.getComps<Transform, Draw>(me);
		data.changeDirTime += deltaTime;
		if (data.changeDirTime > 10.0f)
			data.changeDirTime = -10.0f;

		b.rotation = 360 * data.changeDirTime * 0.1f;
		b.position += Vec2(0, 0.1) * data.changeDirTime * 0.01f;
		d.color += Vec4(rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f);
	}
}
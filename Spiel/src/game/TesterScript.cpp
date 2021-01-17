#include "TesterScript.hpp"

void testerScript(EntityHandle me, Tester& data, float deltaTime)
{
	if (Game::world.hasComps<Transform, Draw>(me)) {
		auto [b, d] = Game::world.getComps<Transform, Draw>(me);
		data.changeDirTime += deltaTime;
		if (data.changeDirTime > 10.0f)
			data.changeDirTime = -10.0f;

		b.position += Vec2(0, 0.1) * data.changeDirTime * 0.01f;
		d.color += Vec4(rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f);
	}
}
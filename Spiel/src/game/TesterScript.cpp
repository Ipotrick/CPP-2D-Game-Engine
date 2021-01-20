#include "TesterScript.hpp"

void testerScript(Game& game, EntityHandle me, Tester& data, float deltaTime)
{
	if (game.world.hasComps<Transform, Draw>(me)) {
		auto [b, d] = game.world.getComps<Transform, Draw>(me);
		data.changeDirTime += deltaTime;
		if (data.changeDirTime > 10.0f)
			data.changeDirTime = -10.0f;

		b.position += Vec2(0, 0.1) * data.changeDirTime * 0.01f;
		d.color += Vec4(rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f, rand() % 1000 / 100000.0f);
	}
}
#include "movementScript.hpp"

void movementScript(Game& game, EntityHandle entity, Transform& t, Movement& m, float deltaTime)
{
	if (fabs(m.velocity.x) + fabs(m.velocity.y) < 0.000001) m.velocity = Vec2(0, 0);
	if (fabs(m.angleVelocity) < 0.000001) m.angleVelocity = 0;
	t.position += m.velocity * deltaTime;
	t.rotaVec = t.rotaVec * RotaVec2(m.angleVelocity * RAD * deltaTime);
}

void movementScriptNarrow(Game& game, EntityHandle entity)
{
	Transform& t = game.world.getComp<Transform>(entity);
	Movement& m = game.world.getComp<Movement>(entity);
	float deltaTime = game.getDeltaTimeSafe();
	if (fabs(m.velocity.x) + fabs(m.velocity.y) < 0.000001) m.velocity = Vec2(0, 0);
	if (fabs(m.angleVelocity) < 0.000001) m.angleVelocity = 0;
	t.position += m.velocity * deltaTime;
	t.rotaVec = t.rotaVec * RotaVec2(m.angleVelocity * RAD * deltaTime);
}

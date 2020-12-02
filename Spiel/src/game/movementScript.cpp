#include "movementScript.hpp"

void movementScript(EntityHandle entity, Transform& t, Movement& m, float deltaTime)
{
	if (fabs(m.velocity.x) + fabs(m.velocity.y) < 0.000001) m.velocity = Vec2(0, 0);
	if (fabs(m.angleVelocity) < 0.000001) m.angleVelocity = 0;
	t.position += m.velocity * deltaTime;
	t.rotation += m.angleVelocity * RAD * deltaTime;
}

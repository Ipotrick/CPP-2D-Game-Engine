#include "MovementSystem.hpp"

void MovementSystem::execute(World& world, float deltaTime)
{
	// execute physics changes in pos, rota:
	for (auto ent : world.entity_view<Movement, Base>()) {
		auto& base = world.getComp<Base>(ent);
		auto& mov = world.getComp<Movement>(ent);
		if (fabs(mov.velocity.x) + fabs(mov.velocity.y) < Physics::nullDelta) mov.velocity = Vec2(0, 0);
		if (fabs(mov.angleVelocity) < Physics::nullDelta) mov.angleVelocity = 0;
		base.position += mov.velocity * deltaTime;
		base.rotation += mov.angleVelocity * deltaTime;
	}
}

void MovementSystem::end()
{
}

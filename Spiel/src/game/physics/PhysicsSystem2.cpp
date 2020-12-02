#include "PhysicsSystem2.hpp"

void PhysicsSystem2::eraseDeadConstraints()
{
	size_t end = collConstraints.size();
	for (size_t i = 0; i < end; i++) {
		if (!collConstraints[i].updated) {
			collConstraints.erase(i);
			//as the last one takes place of th erased element we have to check again against this index
			i--;
			end--;
		}
		else {
			collConstraints[i].updated = false;
		}
	}
}

const std::vector<Drawable>& PhysicsSystem2::getDebugDrawables() const
{
	return debugDrawables;
}

void PhysicsSystem2::applyImpulses(World& world)
{
	for (int i = 0; i < impulseIterations; ++i) {
		for (auto& c : collConstraints) {
			applyImpulse(world, c);
		}
	}
}

void PhysicsSystem2::drawAllCollisionConstraints()
{
	for (auto& c : collConstraints) {
		debugDrawables.push_back(Drawable(0, c.collisionPoints[0].position, 0.95, Vec2(0.05, 0.05), Vec4(1, 0, 0, 1), Form::Circle, RotaVec2(0.0f)));
		debugDrawables.push_back(Drawable(0, c.collisionPoints[1].position, 1, Vec2(0.04, 0.04), Vec4(1, 1, 0, 1), Form::Circle, RotaVec2(0.0f)));
		auto arrow0 = makeArrow(c.collisionPoints[0].normal * 0.05, c.collisionPoints[0].position);
		for (auto el : arrow0)
			debugDrawables.push_back(el);
		auto arrow1 = makeArrow(c.collisionPoints[1].normal * 0.05, c.collisionPoints[1].position);
		for (auto el : arrow1)
			debugDrawables.push_back(el);
	}
}
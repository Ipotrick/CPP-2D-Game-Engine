#include "PhysicsSystem2.hpp"

void PhysicsSystem2::eraseDeadConstraints()
{
	uint32_t end = uint32_t(collConstraints.size());
	for (uint32_t i = 0; i < end; i++) {
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

const std::vector<Sprite>& PhysicsSystem2::getDebugSprites() const
{
	return debugSprites;
}

void PhysicsSystem2::applyImpulses(CollisionSECM world)
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
		debugSprites.push_back(makeSprite(0, c.collisionPoints[0].position, 0.95, Vec2(0.05, 0.05), Vec4(1, 0, 0, 1), Form::Circle, RotaVec2(0.0f)));
		debugSprites.push_back(makeSprite(0, c.collisionPoints[1].position, 1, Vec2(0.04, 0.04), Vec4(1, 1, 0, 1), Form::Circle, RotaVec2(0.0f)));
		auto arrow0 = makeArrow(c.collisionPoints[0].normal * 0.05, c.collisionPoints[0].position);
		for (auto el : arrow0)
			debugSprites.push_back(el);
		auto arrow1 = makeArrow(c.collisionPoints[1].normal * 0.05, c.collisionPoints[1].position);
		for (auto el : arrow1)
			debugSprites.push_back(el);
	}
}

void PhysicsSystem2::updateCollisionConstraints(CollisionSECM world, CollisionSystem& collSys)
{
	for (CollisionInfo collinfo : collSys.collisionInfos) {
		if (world.hasComp<PhysicsBody>(collinfo.indexA) && world.hasComp<PhysicsBody>(collinfo.indexB)) {
			EntityHandle a = world.getHandle(collinfo.indexA);
			EntityHandle b = world.getHandle(collinfo.indexB);
			// order a and b
			collinfo.normal[0] *= -1;			// in physics the normal goes from a to b
			collinfo.normal[1] *= -1;			// in physics the normal goes from a to b
			if (a.index > b.index) {
				std::swap(a, b);
				collinfo.normal[0] *= -1;		// in physics the normal goes from a to b
				collinfo.normal[1] *= -1;		// in physics the normal goes from a to b
				if (collinfo.collisionPointNum > 1) {
					std::swap(collinfo.position[0], collinfo.position[1]);
					std::swap(collinfo.normal[0], collinfo.normal[1]);
				}
			}

			auto* optional = collConstraints.getIfContains(a, b);
			if (optional == nullptr) {
				collConstraints.insert(a, b, collinfo);
			}
			else {
				auto& constraint = *optional;
				if (!constraint.updated) {
					constraint.updated = true;
					constraint.collisionPoints[0].normal = collinfo.normal[0];
					constraint.collisionPoints[1].normal = collinfo.normal[1];
					if (constraint.collisionPointNum != collinfo.collisionPointNum) {
						// reset stored impulses when contact changes dramaticly
						constraint.collisionPoints[0] = CollisionPoint();
						constraint.collisionPoints[1] = CollisionPoint();
						constraint.collisionPointNum = collinfo.collisionPointNum;
					}
					constraint.collisionPoints[0].position = collinfo.position[0];
					constraint.collisionPoints[1].position = collinfo.position[1];
					constraint.clippingDist = collinfo.clippingDist;
				}
			}
		}
	}
}

void PhysicsSystem2::clearDuplicates(CollisionSECM world, CollisionSystem& collSys)
{
	uniqueCollisionInfos.clear();
	uniqueCollisionInfosSetBuffer.clear();

	for (auto const& el : collSys.getCollisions()) {
		const EntityHandleIndex a = std::min(el.indexA, el.indexB);
		const EntityHandleIndex b = std::max(el.indexA, el.indexB);
		uint64_t key = makeConstraintKey(a, b);
		if (!uniqueCollisionInfosSetBuffer.contains(key)) {
			uniqueCollisionInfosSetBuffer.insert(key);
			uniqueCollisionInfos.push_back(el);
		}
	}
}

void PhysicsSystem2::findIslands(CollisionSECM world, CollisionSystem& collSys)
{
	std::vector<CollisionConstraintSet> disjointSetList;
	CollisionConstraintSet leftOverConstraints = collConstraints; 

	constexpr int DISJOINT_SET_COUNT{ 5 };


	for (int i = 0; i < DISJOINT_SET_COUNT; i++) {
		CollisionConstraintSet disjointSet;
		robin_hood::unordered_set<EntityHandleIndex> invalidSet;

		for (auto const& c : leftOverConstraints) {
			const EntityHandle a = c.idA.index < c.idB.index ? c.idA : c.idB;
			const EntityHandle b = c.idA.index < c.idB.index ? c.idB : c.idA;

			if (!invalidSet.contains(a.index) && !invalidSet.contains(b.index)) {
				disjointSet.insert(a, b, c);
				leftOverConstraints.erase(a, b);
				invalidSet.insert(a.index);
				invalidSet.insert(b.index);
			}
		}
		
		disjointSetList.push_back(std::move(disjointSet));
	}

	Vec4 colors[10] = {
		{0,0,0,1},
		{0,0,1,1},
		{0,1,0,1},
		{0,1,1,1},
		{1,0,0,1},
		{1,0,1,1},
		{1,1,0,1},
		{1,1,1,1},
		{0.5,0.5,0.5,1},
		{0.5,1,0,1}
	};

	//int i = 0;
	//for (auto & el : disjointSetList) {
	//	std::cout << "islandset " << i << " has " << el.size() << " elements" << std::endl;
	//
	//	for (auto const& c : el) {
	//		auto p1 = world.getComp<Transform>(c.idA).position;
	//		auto p2 = world.getComp<Transform>(c.idB).position;
	//		auto sprites = makeArrow(p2 - p1, p1, colors[i]);
	//		for (auto s : sprites) {
	//			EngineCore::renderer.submit(s, LAYER_WORLD_FOREGROUND);
	//		}
	//	}
	//	++i;
	//}
}

void PhysicsSystem2::prepareConstraints(CollisionSECM world, float deltaTime)
{
	const float k_allowedPenetration = 0.01f;
	float k_biasFactor = positionCorrection ? 0.2f : 0.0f;

	for (auto& c : collConstraints) {
		const EntityHandle entA = c.idA;
		const EntityHandle entB = c.idB;
		auto movementDummy = Movement();
		auto& baseA = world.getComp<Transform>(entA);
		auto& moveA = world.hasComp<Movement>(entA) ? world.getComp<Movement>(entA) : movementDummy;
		auto& bodyA = world.getComp<PhysicsBody>(entA);
		auto& baseB = world.getComp<Transform>(entB);
		auto& moveB = world.hasComp<Movement>(entB) ? world.getComp<Movement>(entB) : movementDummy;
		auto& bodyB = world.getComp<PhysicsBody>(entB);

		c.friction = sqrt(bodyA.friction * bodyB.friction);
		float restitution = 1.0f + std::max(bodyA.elasticity, bodyB.elasticity);
		c.bias = -k_biasFactor * (1.0f / deltaTime) * std::min(0.0f, -c.clippingDist + k_allowedPenetration);

		for (int i = 0; i < c.collisionPointNum; i++) {
			Vec2 tangent = rotate<270>(c.collisionPoints[i].normal);
			Vec2 r1 = c.collisionPoints[i].position - baseA.position;
			Vec2 r2 = c.collisionPoints[i].position - baseB.position;

			float rn1 = dot(r1, c.collisionPoints[i].normal);
			float rn2 = dot(r2, c.collisionPoints[i].normal);
			float kNormal = (1.0f / bodyA.mass) + (1.0f / bodyB.mass);
			kNormal += (1.0f / bodyA.momentOfInertia) * (dot(r1, r1) - rn1 * rn1) + (1.0f / bodyB.momentOfInertia) * (dot(r2, r2) - rn2 * rn2);
			c.collisionPoints[i].massNormal = 1.0f / kNormal;

			float rt1 = dot(r1, tangent);
			float rt2 = dot(r2, tangent);
			float kTangent = (1 / bodyA.mass) + (1 / bodyB.mass);
			kTangent += (1 / bodyA.momentOfInertia) * (dot(r1, r1) - rt1 * rt1) + (1 / bodyB.momentOfInertia) * (dot(r2, r2) - rt2 * rt2);
			c.collisionPoints[i].massTangent = 1.0f / kTangent;

			if (accumulateImpulses) {
				// Apply normal + friction impulse
				Vec2 P = (c.collisionPoints[i].accPn * c.collisionPoints[i].normal + c.collisionPoints[i].accPt * tangent) * restitution;

				moveA.velocity -= (1.0f / bodyA.mass) * P;
				moveA.angleVelocity -= (1.0f / bodyA.momentOfInertia) * cross(r1, P);

				moveB.velocity += (1.0f / bodyB.mass) * P;
				moveB.angleVelocity += (1.0f / bodyB.momentOfInertia) * cross(r2, P);
			}
		}
	}
}

PhysicsSystem2::PhysicsSystem2()
{
	disjointPairs.reserve(MAX_LAYERS);
}

void PhysicsSystem2::springyPositionCorrection(CollisionSECM world, float deltaTime)
{
	auto springForce = [](float overlap) -> float {
		overlap = clamp(overlap, 0.0f, 0.2f) * 5.0f;
		return overlap * overlap * 0.01f;
	};

	for (auto& c : collConstraints) {
		if (world.hasComps<Movement>(c.idA)) {
			world.getComp<Transform>(c.idA).position -= (c.collisionPoints[0].normal + c.collisionPoints[1].normal) * springForce(c.clippingDist);
		}
		if (world.hasComps<Movement>(c.idB)) {
			world.getComp<Transform>(c.idB).position += (c.collisionPoints[0].normal + c.collisionPoints[1].normal) * springForce(c.clippingDist);
		}
	}
}

void PhysicsSystem2::applyImpulse(CollisionSECM world, CollisionConstraint& c)
{
	const EntityHandle entA = c.idA;
	const EntityHandle entB = c.idB;
	auto movementDummy = Movement();
	auto& baseA = world.getComp<Transform>(entA);
	auto& moveA = world.hasComp<Movement>(entA) ? world.getComp<Movement>(entA) : movementDummy;
	auto& bodyA = world.getComp<PhysicsBody>(entA);
	auto& baseB = world.getComp<Transform>(entB);
	auto& moveB = world.hasComp<Movement>(entB) ? world.getComp<Movement>(entB) : movementDummy;
	auto& bodyB = world.getComp<PhysicsBody>(entB);

	for (int i = 0; i < c.collisionPointNum; ++i) {

		Vec2 ra = c.collisionPoints[i].position - baseA.position;
		Vec2 rb = c.collisionPoints[i].position - baseB.position;

		// Relative velocity at contact
		Vec2 dv = moveB.velocity + cross(moveB.angleVelocity, rb) - moveA.velocity - cross(moveA.angleVelocity, ra);

		// Compute normal impulse
		float vn = dot(dv, c.collisionPoints[i].normal);

		float dPn = c.collisionPoints[i].massNormal * (-vn + c.bias);

		if (accumulateImpulses) {
			// Clamp the accumulated impulse
			float Pn0 = c.collisionPoints[i].accPn;
			c.collisionPoints[i].accPn = std::max(Pn0 + dPn, 0.0f);
			dPn = c.collisionPoints[i].accPn - Pn0;
		}
		else {
			dPn = std::max(dPn, 0.0f);
		}

		// Apply contact impulse
		Vec2 Pn = dPn * c.collisionPoints[i].normal;

		moveA.velocity -= (1.0f / bodyA.mass) * Pn;
		moveA.angleVelocity -= (1.0f / bodyA.momentOfInertia) * cross(ra, Pn);

		moveB.velocity += (1.0f / bodyB.mass) * Pn;
		moveB.angleVelocity += (1.0f / bodyB.momentOfInertia) * cross(rb, Pn);

		//============================================FRICTION=====================================================//

				// Relative velocity at contact
		dv = moveB.velocity + cross(moveB.angleVelocity, rb) - moveA.velocity - cross(moveA.angleVelocity, ra);

		Vec2 tangent = rotate<270>(c.collisionPoints[i].normal);
		float vt = dot(dv, tangent);
		float dPt = c.collisionPoints[i].massTangent * (-vt);

		if (accumulateImpulses) {
			// Compute friction impulse
			float maxPt = c.friction * c.collisionPoints[i].accPn;

			// Clamp friction
			float oldTangentImpulse = c.collisionPoints[i].accPt;
			c.collisionPoints[i].accPt = clamp(oldTangentImpulse + dPt, -maxPt, maxPt);
			dPt = c.collisionPoints[i].accPt - oldTangentImpulse;
		}
		else {
			float maxPt = c.friction * dPn;
			dPt = clamp(dPt, -maxPt, maxPt);
		}

		// Apply contact impulse
		Vec2 Pt = dPt * tangent;

		moveA.velocity -= (1.0f / bodyA.mass) * Pt;
		moveA.angleVelocity -= (1.0f / bodyA.momentOfInertia) * cross(ra, Pt);

		moveB.velocity += (1.0f / bodyB.mass) * Pt;
		moveB.angleVelocity += (1.0f / bodyB.momentOfInertia) * cross(rb, Pt);
	}
}

void PhysicsSystem2::applyForcefields(CollisionSECM world, PhysicsUniforms const& uniform, float deltaTime)
{
	for (auto [ent, p, mov, base] : world.entityComponentView<PhysicsBody, Movement, Transform>()) {
		mov.velocity += uniform.linearEffectDir * uniform.linearEffectAccel * deltaTime;
		mov.velocity += uniform.linearEffectDir * uniform.linearEffectForce * (1.0f / world.getComp<PhysicsBody>(ent).mass) * deltaTime;
		mov.velocity *= (1.0f - uniform.friction * deltaTime);
		mov.angleVelocity *= (1.0f - uniform.friction * deltaTime);
	}
}

void PhysicsSystem2::execute(CollisionSECM world, PhysicsUniforms const& uniform, float deltaTime, CollisionSystem& collSys)
{
	deltaTime = std::min(deltaTime, minDelaTime);
	debugSprites.clear();

	//LOG_FUNCTION_TIME("clearDuplicates",clearDuplicates(world, collSys));
	updateCollisionConstraints(world, collSys);
	eraseDeadConstraints();
	if (positionCorrection) springyPositionCorrection(world, deltaTime);
	prepareConstraints(world, deltaTime);
	applyImpulses(world);
	applyForcefields(world, uniform, deltaTime);
	//drawAllCollisionConstraints();
}
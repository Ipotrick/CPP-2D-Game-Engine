#include "PhysicsSystem2.hpp"
#include "debug.hpp"
#include "log/Log.hpp"

PhysicsSystem2::PhysicsSystem2(JobManager& jobs, PerfLogger& perf)
	:jobManager{ jobs }, perfLog{ perf }
{
	disjointPairs.reserve(MAX_LAYERS);
}

void PhysicsSystem2::execute(World& world, float deltaTime, CollisionSystem& collSys)
{
	Timer t(perfLog.getInputRef("physicsprepare"));
	deltaTime = std::min(deltaTime, minDelaTime);
	debugDrawables.clear();
	updateCollisionConstraints(world, collSys);
	eraseDeadConstraints();
	t.stop();
	if (positionCorrection) springyPositionCorrection(world, deltaTime);
	prepareConstraints(world, deltaTime);
	applyImpulses(world);
	applyForcefields(world, deltaTime);
	//drawAllCollisionConstraints();
}

const std::vector<Drawable>& PhysicsSystem2::getDebugDrawables() const
{
	return debugDrawables;
}

void PhysicsSystem2::updateCollisionConstraints(World& world, CollisionSystem& collSys)
{
	for (CollisionInfo collinfo : collSys.collisionInfos) {
		if (world.hasComp<PhysicsBody>(collinfo.indexA) && world.hasComp<PhysicsBody>(collinfo.indexB)) {
			EntityHandle a = world.getHandle(collinfo.indexA);
			EntityHandle b = world.getHandle(collinfo.indexB);
				// order a and b
			collinfo.normal[0] *= -1;			// in physics the normal goes from a to b
			collinfo.normal[1] *= -1;			// in physics the normal goes from a to b
			if (a.index > b.index) {
				const auto temp = a;
				a = b;
				b = temp;
				collinfo.normal[0] *= -1;	// in physics the normal goes from a to b
				collinfo.normal[1] *= -1;	// in physics the normal goes from a to b
				if (collinfo.collisionPointNum > 1) {
					const auto temp2 = collinfo.position[0];
					collinfo.position[0] = collinfo.position[1];
					collinfo.position[1] = temp2;
					const auto temp3 = collinfo.normal[0];
					collinfo.normal[0] = collinfo.normal[1];
					collinfo.normal[1] = temp3;
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
						// reset sotred impulses when contact changes dramaticly
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

void PhysicsSystem2::eraseDeadConstraints()
{
	int end = collConstraints.size();
	for (int i = 0; i < end; i++) {
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

void PhysicsSystem2::prepareConstraints(World& world, float deltaTime)
{
	const float k_allowedPenetration = 0.01f;
	float k_biasFactor = positionCorrection ? 0.2f : 0.0f;

	for (auto& c : collConstraints) {
		const EntityHandle entA = c.idA;
		const EntityHandle entB = c.idB;
		auto movementDummy = Movement();
		auto& baseA = world.getComp<Transform>(entA);
		auto& collA = world.getComp<Collider>(entA);
		auto& moveA = world.hasComp<Movement>(entA) ? world.getComp<Movement>(entA) : movementDummy;
		auto& bodyA = world.getComp<PhysicsBody>(entA);
		auto& baseB = world.getComp<Transform>(entB);
		auto& collB = world.getComp<Collider>(entB);
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

void PhysicsSystem2::springyPositionCorrection(World& world, float deltaTime)
{
	auto springForce = [](float overlap) {
		overlap = clamp(overlap, 0.0f, 0.2f) * 5;
		return overlap * overlap * 0.01;
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

void PhysicsSystem2::applyImpulse(World& world, CollisionConstraint& c)
{
	const EntityHandle entA = c.idA;
	const EntityHandle entB = c.idB;
	auto movementDummy = Movement();
	auto& baseA = world.getComp<Transform>(entA);
	auto& collA = world.getComp<Collider>(entA);
	auto& moveA = world.hasComp<Movement>(entA) ? world.getComp<Movement>(entA) : movementDummy;
	auto& bodyA = world.getComp<PhysicsBody>(entA);
	auto& baseB = world.getComp<Transform>(entB);
	auto& collB = world.getComp<Collider>(entB);
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

void PhysicsSystem2::applyImpulses(World& world)
{
	Timer t3(perfLog.getInputRef("physicsimpulse"));
	for (int i = 0; i < impulseIterations; ++i) {
		for (auto& c : collConstraints) {
			applyImpulse(world, c);
		}
	}
}

void PhysicsSystem2::applyImpulsesMultiThreadded(World& world)
{
	Timer t3(perfLog.getInputRef("physicsimpulse"));
	std::vector<LambdaJob> jobs;
	std::vector<Tag> jobTags;

	visited.resize(world.maxEntityIndex() + 1, false);
	used.clear();
	used.resize(collConstraints.size() + 1, false);
	for (auto& d : disjointPairs) {
		d.clear();
	}
	restConstraints.clear();

	int size = 0;

	int dpi = 0;
	do {
		disjointPairs.push_back(std::vector<CollisionConstraint*>());
		size = 0;
		visited.clear();
		visited.resize(world.maxEntityIndex() + 1, false);
		for (int i = 0; i < collConstraints.size(); i++) {
			if (!used[i] && !visited[(collConstraints[i].idA).index] & !visited[(collConstraints[i].idB).index]) {
				disjointPairs[dpi].push_back(&collConstraints[i]);
				visited[(collConstraints[i].idA).index] = true;
				visited[(collConstraints[i].idB).index] = true;
				used[i] = true;
				size++;
			}
		}

		if (size < 500 || dpi == MAX_LAYERS) {
			for (auto& c : disjointPairs[dpi]) {
				restConstraints.push_back(c);
			}
			for (int i = 0; i < collConstraints.size(); i++) {
				if (!used[i]) {
					restConstraints.push_back(&collConstraints[i]);
				}
			}
		}

		for (auto& c : restConstraints) {
			applyImpulse(world, *c);
		}

		jobManager.waitAndHelp(&jobTags);

		if (size < 500 || dpi == MAX_LAYERS)
			break;

		jobTags.clear();
		jobs.clear();
		
		int jobCount = disjointPairs[dpi].size() / CONSTRAINTS_PER_JOB + 1;
		for (int i = 0; i < jobCount; i++) {
			int begin = i * CONSTRAINTS_PER_JOB;
			int end = (i + 1) * CONSTRAINTS_PER_JOB;
			jobs.push_back(LambdaJob([&world, this, begin, end, dpi](int workerId) {
				for (int j = begin; j < disjointPairs[dpi].size() && j < end; j++) {
					applyImpulse(world, *disjointPairs[dpi][j]);
				}
				}));
		}
		
		for (auto& job : jobs) {
			jobTags.push_back(jobManager.addJob(&job));
		}
		dpi++;
	} while (true);

	//for (int i = 1; i < impulseIterations; i++) {
	//	for (int dpi = 0; dpi < disjointPairs.size(); dpi++) {
	//		jobTags.clear();
	//		jobs.clear();
	//
	//		int jobCount = disjointPairs[dpi].size() / CONSTRAINTS_PER_JOB + 1;
	//		for (int i = 0; i < jobCount; i++) {
	//			int begin = i * CONSTRAINTS_PER_JOB;
	//			int end = (i + 1) * CONSTRAINTS_PER_JOB;
	//			jobs.push_back(LambdaJob([&world, this, begin, end, dpi](int workerId) {
	//				for (int j = begin; j < disjointPairs[dpi].size() && j < end; j++) {
	//					applyImpulse(world, *disjointPairs[dpi][j]);
	//				}
	//				}));
	//		}
	//		for (auto& job : jobs) {
	//			jobTags.push_back(jobManager.addJob(&job));
	//		}
	//		jobManager.waitAndHelp(&jobTags);
	//	}
	//	for (auto& c : restConstraints) {
	//		applyImpulse(world, *c);
	//	}
	//}
}

void PhysicsSystem2::applyForcefields(World& world, float deltaTime)
{
	for (auto [ent, p, mov, base] : world.entityComponentView<PhysicsBody, Movement, Transform>()) {
		mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectAccel * deltaTime;
		mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectForce * (1.0f / world.getComp<PhysicsBody>(ent).mass) * deltaTime;
		mov.velocity *= (1.0f - world.physics.friction * deltaTime);
		mov.angleVelocity *= (1.0f - world.physics.friction * deltaTime);
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

#include "PhysicsSystem.hpp"
#include <set>

#include "PushoutCalcJob.hpp"

//#define DEBUG_PRESSURE

PhysicsSystem::PhysicsSystem(World& world, JobManager& jobManager, PerfLogger& perfLog) :
	CoreSystem( world ),
	jobManager{ jobManager },
	perfLog{ perfLog }
{
}

void PhysicsSystem::execute(World& world, float deltaTime, CollisionSystem& collSys)
{
	debugDrawables.clear();
	applyPhysics(deltaTime, collSys);
}

void PhysicsSystem::end()
{
}

void PhysicsSystem::applyPhysics(float deltaTime, CollisionSystem& collSys)
{
	Timer t3(perfLog.getInputRef("physicsexecute"));

	velocityBuffer.clear();
	velocityBuffer.resize(world.memorySize());
	overlapAccumBuffer.clear();
	overlapAccumBuffer.resize(world.memorySize());
	for (auto ent : world.index_view<Movement, PhysicsBody>()) {
		velocityBuffer[ent] = world.getComp<Movement>(ent).velocity;
		overlapAccumBuffer[ent] = world.getComp<PhysicsBody>(ent).overlapAccum;
	}

	PushoutCalcJob pushOutCalcJob = PushoutCalcJob(collSys.getAllCollisions(), velocityBuffer, overlapAccumBuffer, collSys.poolWorkerData->collisionResponses, world);
	auto pushOutCalcJobTag = jobManager.addJob(&pushOutCalcJob);

// collision info operations begin:
	for (auto ent : world.index_view<PhysicsBody>()) {
		world.getComp<PhysicsBody>(ent).overlapAccum *= 1 - (Physics::pressureFalloff * deltaTime);
	}
	// execute inelastic collisions 
	for (int i = 0; i < impulseResulutionIterations; i++) {
		for (auto& collInfo : collSys.getAllCollisions()) {
			uint32_t entA = collInfo.idA;
			uint32_t entB = collInfo.idB;


			if (world.hasComp<PhysicsBody>(entA) & world.hasComp<PhysicsBody>(entB)) { //check if both are solid
				if (!world.hasComp<Movement>(entB)) {
					world.getComp<PhysicsBody>(entA).overlapAccum += collInfo.clippingDist * 2.0f;	// collision with wall makes greater pressure
				}
				else {
					world.getComp<PhysicsBody>(entA).overlapAccum += collInfo.clippingDist;
				}

				// owner stands in place for the slave for a collision response execution
				if (world.hasComp<BaseChild>(entA) | world.hasComp<BaseChild>(entB)) {
					if (world.hasComp<BaseChild>(entA) && !world.hasComp<BaseChild>(entB)) {
						entA = world.getIndex(world.getComp<BaseChild>(entA).parent);
					}
					else if (!world.hasComp<BaseChild>(entA) && world.hasComp<BaseChild>(entB)) {
						entB = world.getIndex(world.getComp<BaseChild>(entB).parent);
					}
					else {
						// both are slaves
						entA = world.getIndex(world.getComp<BaseChild>(entA).parent);
						entB = world.getIndex(world.getComp<BaseChild>(entB).parent);
					}
				}

				if (world.hasComp<PhysicsBody>(entA) & world.hasComp<PhysicsBody>(entB)) { // recheck if the owners are solid
					auto& solidA = world.getComp<PhysicsBody>(entA);
					auto& baseA = world.getComp<Base>(entA);
					auto& moveA = world.getComp<Movement>(entA);
					auto& solidB = world.getComp<PhysicsBody>(entB);
					auto& baseB = world.getComp<Base>(entB);
					Movement dummy = Movement();
					Movement& moveB = (world.hasComp<Movement>(entB) ? world.getComp<Movement>(entB) : dummy);

					

					auto& collidB = world.getComp<Collider>(entB);

					float elast = std::max(solidA.elasticity, solidB.elasticity);
					float friction = std::min(solidA.friction, solidB.friction) * deltaTime;
					auto [collChanges, otherChanges] = impulseResolution(
						baseA.position, moveA.velocity, moveA.angleVelocity, solidA.mass, solidA.momentOfInertia,
						baseB.position, moveB.velocity, moveB.angleVelocity, solidB.mass, solidB.momentOfInertia,
						collInfo.collisionNormal, collInfo.collisionPos, elast, friction);
					moveA.velocity += collChanges.first;
					moveA.angleVelocity += collChanges.second;
					moveB.velocity += otherChanges.first;
					moveB.angleVelocity += otherChanges.second;

				}
			}
		}
	}

	jobManager.waitFor(pushOutCalcJobTag);

	// let entities sleep or wake them up
	for (auto entity : world.index_view<Movement, Collider, Base>()) {
		auto& collider = world.getComp<Collider>(entity);
		auto& movement = world.getComp<Movement>(entity);
		auto& base = world.getComp<Base>(entity);

		if (movement.angleVelocity == 0 && movement.velocity == Vec2(0,0) && !collider.particle) {
			collider.sleeping = true;
		}
		else {
			collider.sleeping = false;
		}
	}

// collision info operations end!
// effector operations begin:
	// linear effector execution
	for (auto ent : world.index_view<LinearEffector>()) {
		auto& moveField = world.getComp<LinearEffector>(ent);
		auto [begin, end] = collSys.getCollisions(ent);
		for (auto iter = begin; iter != end; ++iter) {
			if (world.hasComps<Movement, PhysicsBody>(iter->idB)) {
				auto& mov = world.getComp<Movement>(iter->idB);
				auto& solid = world.getComp<PhysicsBody>(iter->idB);
				mov.velocity += moveField.movdir * moveField.acceleration * deltaTime;
				mov.velocity += moveField.movdir * moveField.force / solid.mass * deltaTime;
			}
		}
	}

	// friction effector execution
	for (auto ent : world.index_view<FrictionEffector>()) {
		auto& moveField = world.getComp<FrictionEffector>(ent);
		auto [begin, end] = collSys.getCollisions(ent);
		for (auto iter = begin; iter != end; ++iter) {
			if (world.hasComps<Movement, PhysicsBody>(iter->idB)) {
				world.getComp<Movement>(iter->idB).velocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).friction));
				world.getComp<Movement>(iter->idB).angleVelocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).rotationalFriction));
			}
		}
	}

	// uniform effector execution :
	for (auto ent : world.index_view<Movement, PhysicsBody>()) {
		auto& mov = world.getComp<Movement>(ent);
		auto& solid = world.getComp<PhysicsBody>(ent);
		mov.velocity *= (1 / (1 + deltaTime * std::min(world.physics.friction, solid.friction)));
		mov.angleVelocity *= (1 / (1 + deltaTime * std::min(world.physics.friction, solid.friction)));
		mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectAccel * deltaTime;
		mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectForce / solid.mass * deltaTime;
	}
// effector operations end!
// velocity and pushout operations:

	// apply dampened collision response pushout of slave to owner
	propagateChildPushoutToParent(collSys);

	// apply pushout
	for (auto ent : world.index_view<Movement, PhysicsBody, Base>()) {
		auto& base = world.getComp<Base>(ent);
		base.position += collSys.poolWorkerData->collisionResponses[ent].posChange;
	}

	// execute physics changes in pos, rota
	for (auto ent : world.index_view<Movement, Base>()) {

		auto& movement = world.getComp<Movement>(ent);
		auto& base = world.getComp<Base>(ent);

		if (fabs(movement.velocity.x) + fabs(movement.velocity.y) < Physics::nullDelta) movement.velocity = Vec2(0, 0);
		if (fabs(movement.angleVelocity) < Physics::nullDelta) movement.angleVelocity = 0;

		base.position += movement.velocity * deltaTime;
		base.rotation += movement.angleVelocity * deltaTime;

	}
// velocity and pushouts end!
	syncBaseChildrenToParents();
	t3.stop();
	// submit debug drawables for physics
	for (auto& el : Physics::debugDrawables) {
		debugDrawables.push_back(el);
	}
#ifdef DEBUG_PRESSURE

	float allClipping = 0.0f;
	float maxClipping = 0.0f;
	for (auto ent : world.view<PhysicsBody, Draw>()) {
		auto& phys = world.getComp<PhysicsBody>(ent);
		maxClipping = std::max(maxClipping, phys.overlapAccum);
		allClipping += phys.overlapAccum;
	}
	for (auto ent : world.view<PhysicsBody, Draw>()) {
		auto& draw = world.getComp<Draw>(ent);
		auto& phys = world.getComp<PhysicsBody>(ent);

		auto otherColors = std::min(phys.overlapAccum / maxClipping, 1.0f);
		draw.color.r = 1;
		draw.color.g = 1.0f - otherColors;
		draw.color.b = 1.0f - otherColors;
	}
	std::cout << "clipping: " << allClipping << std::endl;
#endif

#ifdef DEBUG_QUADTREE
	std::vector<Drawable> debug;
	for (auto el : world.view<Player>()) {
		PosSize posSize(world.getComp<Base>(el).position, aabbBounds(world.getComp<Collider>(el).size, world.getComp<Base>(el).rotaVec));
		poolWorkerData->qtreeDynamic.querryDebug(posSize, debug);
		debugDrawables.push_back(Drawable(0, world.getComp<Base>(el).position, 0.25f, aabbBounds(world.getComp<Collider>(el).size, world.getComp<Base>(el).rotaVec), Vec4(1, 0, 0, 1), Form::RECTANGLE, 0));
	}
	for (auto el : debug) debugDrawables.push_back(el);
#endif
#ifdef DEBUG_QUADTREE2

	std::vector<Drawable> debug2;
	poolWorkerData->qtreeDynamic.querryDebugAll(debug2, Vec4(1, 0, 1, 1));
	for (auto el : debug2) debugDrawables.push_back(el);
#endif
#ifdef DEBUG_COLLIDER_SLEEP
	for (auto ent : world.view<Movement, PhysicsBody, Draw>()) {
		auto& collider = world.getComp<Collider>(ent);
		auto& draw = world.getComp<Draw>(ent);
		if (collider.sleeping) {
			draw.color.r = 0.1f;
			draw.color.g = 0.3f;
			draw.color.b = 0.3f;
		}
		else {
			draw.color.r = 1;
			draw.color.g = 1;
			draw.color.b = 1;
		}
	}
#endif
#ifdef _DEBUG
	// a particle can never sleep as it could not be waken up by collisions
	for (auto ent : world.view<Collider>()) {
		auto& collider = world.getComp<Collider>(ent);
		assert(!(collider.particle && collider.sleeping));
	}
#endif
}

void PhysicsSystem::propagateChildPushoutToParent(CollisionSystem& collSys)
{
	for (auto child : world.index_view<BaseChild, PhysicsBody>()) {
		auto relationship = world.getComp<BaseChild>(child);
		auto parent = world.getIndex(relationship.parent);
		float childWeight = norm(collSys.poolWorkerData->collisionResponses[child].posChange);
		float parentWeight = norm(collSys.poolWorkerData->collisionResponses[parent].posChange);
		float normalizer = childWeight + parentWeight;
		if (normalizer > Physics::nullDelta) {
			collSys.poolWorkerData->collisionResponses[parent].posChange = (childWeight * collSys.poolWorkerData->collisionResponses[child].posChange + parentWeight * collSys.poolWorkerData->collisionResponses[parent].posChange) / normalizer;
		}
	}
}

void PhysicsSystem::syncBaseChildrenToParents() {
	for (auto child : world.index_view<BaseChild>()) {
		auto& base = world.getComp<Base>(child);
		auto& movement = world.getComp<Movement>(child);
		auto& relationship = world.getComp<BaseChild>(child);

		auto parent = world.getIndex(relationship.parent);
		auto& baseP = world.getComp<Base>(parent);
		auto& movementP = world.getComp<Movement>(parent);

		base.position = baseP.position + rotate(relationship.relativePos, baseP.rotation);
		movement.velocity = movementP.velocity;
		base.rotation = baseP.rotation + relationship.relativeRota;
		movement.angleVelocity = movementP.angleVelocity;
	}
}
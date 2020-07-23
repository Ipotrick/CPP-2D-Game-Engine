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


void PhysicsSystem::findIslands(float deltaTime, CollisionSystem& collSys) {
	entityIslandMarks.clear();
	entityIslandMarks.resize(world.memorySize(), 0);

	int islandCount = 1;
	for (auto collInfo : collSys.indexCollisionInfos) {
		if (entityIslandMarks[collInfo.indexB] > 0) {
		entityIslandMarks[collInfo.indexA] = entityIslandMarks[collInfo.indexB];
		}
		else if (entityIslandMarks[collInfo.indexA] > 0) {
			entityIslandMarks[collInfo.indexB] = entityIslandMarks[collInfo.indexA];
		}
		else {
			entityIslandMarks[collInfo.indexA] = entityIslandMarks[collInfo.indexB] = islandCount;
			islandCount++;
		}
	}
	for (auto collInfo : collSys.indexCollisionInfos) {
		if (entityIslandMarks[collInfo.indexB] > entityIslandMarks[collInfo.indexA]) {
			entityIslandMarks[collInfo.indexB] = entityIslandMarks[collInfo.indexA];
		}
		else if (entityIslandMarks[collInfo.indexB] < entityIslandMarks[collInfo.indexA]) {
			entityIslandMarks[collInfo.indexA] = entityIslandMarks[collInfo.indexB];
		}
	}
	int borderCollisions = 0;
	int allCollisions = 0;
	for (auto collInfo : collSys.indexCollisionInfos) {
		if (entityIslandMarks[collInfo.indexA] != entityIslandMarks[collInfo.indexB]) borderCollisions++;
		allCollisions++;
	}
#ifdef DEBUG_ISLANDS
	std::array<Vec4, 10> colors{
		Vec4(1,1,0,1),
		Vec4(1,0,1,1),
		Vec4(0,1,1,1),
		Vec4(0.75,0.25,0,1),
		Vec4(0,1,0,1),
		Vec4(0,0,1,1),
		Vec4(1,1,0.5,1),
		Vec4(1,0.5,1,1),
		Vec4(0.5,1,1,1),
		Vec4(0.7,0.7,0.5,1),
	};
	for (auto ent : world.index_view<PhysicsBody, Movement>()) {
		world.getComp<Draw>(ent).color = colors[entityIsland[ent] % 10];
	}
	
	printf("collisions inside of islands: %i, collisions between islands: %i, ratio: %f\n", allCollisions- borderCollisions, borderCollisions, (float)(allCollisions - borderCollisions) / (float)borderCollisions);
#endif
	for (auto& batch : collInfoIslands) 
		batch.clear();
	collInfoIslands.resize(islandCount);
	collInfoIslandsBorder.clear();
	for (auto collInfo : collSys.indexCollisionInfos) {
		if (entityIslandMarks.at(collInfo.indexA) == entityIslandMarks.at(collInfo.indexB)) {
			collInfoIslands[entityIslandMarks[collInfo.indexA]].push_back(collInfo);
		}
		else {
			collInfoIslandsBorder.push_back(collInfo);
		}
	}

	const int maxCollisionCountInBatch = std::max(allCollisions / impulseResolutionJobCount,1);
	int currentBatchCollCount = 0;
	int currentIslandHead = 1;

	islandBatches.clear();
	islandBatches.reserve(islandCount / maxCollisionCountInBatch + 1);
	for (int island = 1; island < islandCount; ++island) {
		currentBatchCollCount += collInfoIslands.at(island).size();
		if (currentBatchCollCount >= maxCollisionCountInBatch) {
			islandBatches.push_back(std::pair(currentIslandHead, island));
			currentIslandHead = island + 1;
			currentBatchCollCount = 0;
		}
	}
	if (currentIslandHead != islandCount) {
		islandBatches.push_back(std::pair(currentIslandHead, islandCount-1));
	}
}

void PhysicsSystem::applyPhysics(float deltaTime, CollisionSystem& collSys)
{
	findIslands(deltaTime, collSys);

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

	for (auto ent : world.index_view<PhysicsBody>()) {
		world.getComp<PhysicsBody>(ent).overlapAccum *= 1 - (Physics::pressureFalloff * deltaTime);
	}

	auto collisionResolution = [&](IndexCollisionInfo& collInfo) {
		uint32_t entA = collInfo.indexA;
		uint32_t entB = collInfo.indexB;

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
	};

	// execute inelastic collisions 
	for (int i = 0; i < impulseResulutionIterations; i++) {
		resolutionJobs.clear(),
		resolutionJobs.reserve(collInfoIslands.size());
		resolutionJobTags.clear();
		resolutionJobTags.reserve(collInfoIslands.size());
		for (auto& batch : islandBatches) {
			resolutionJobs.push_back(CollisionResolutionJob(world, deltaTime, collInfoIslands, batch));
			resolutionJobTags.push_back(jobManager.addJob(&resolutionJobs.back()));
		}
		for (auto tag : resolutionJobTags) {
			jobManager.waitFor(tag);
		}

		for (auto& collInfo : collInfoIslandsBorder) {
			collisionResolution(collInfo);
		}
	}

	jobManager.waitFor(pushOutCalcJobTag);
	propagateChildPushoutToParent(collSys);

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

	// linear effector execution
	for (auto ent : world.index_view<LinearEffector>()) {
		auto& moveField = world.getComp<LinearEffector>(ent);
		auto [begin, end] = collSys.getCollisions(ent);
		for (auto iter = begin; iter != end; ++iter) {
			if (world.hasComps<Movement, PhysicsBody>(iter->indexB)) {
				auto& mov = world.getComp<Movement>(iter->indexB);
				auto& solid = world.getComp<PhysicsBody>(iter->indexB);
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
			if (world.hasComps<Movement, PhysicsBody>(iter->indexB)) {
				world.getComp<Movement>(iter->indexB).velocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).friction));
				world.getComp<Movement>(iter->indexB).angleVelocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).rotationalFriction));
			}
		}
	}
	
	for (auto ent : world.index_view<Movement, Base>()) {
		auto& mov = world.getComp<Movement>(ent);
		if (world.hasComp<PhysicsBody>(ent)) {
			// uniform effector execution :
			auto& solid = world.getComp<PhysicsBody>(ent);
			mov.velocity *= (1 / (1 + deltaTime * std::min(world.physics.friction, solid.friction)));
			mov.angleVelocity *= (1 / (1 + deltaTime * std::min(world.physics.friction, solid.friction)));
			mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectAccel * deltaTime;
			mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectForce / solid.mass * deltaTime;
		}
		// apply pushout:
		auto& base = world.getComp<Base>(ent);
		base.position += collSys.poolWorkerData->collisionResponses[ent].posChange;
		// execute physics changes in pos, rota:
		if (fabs(mov.velocity.x) + fabs(mov.velocity.y) < Physics::nullDelta) mov.velocity = Vec2(0, 0);
		if (fabs(mov.angleVelocity) < Physics::nullDelta) mov.angleVelocity = 0;
		base.position += mov.velocity * deltaTime;
		base.rotation += mov.angleVelocity * deltaTime;
	}

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
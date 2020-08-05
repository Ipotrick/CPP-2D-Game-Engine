#include "PhysicsSystem.hpp"
#include <set>

#include "PushoutCalcJob.hpp"


PhysicsSystem::PhysicsSystem(JobManager& jobManager, PerfLogger& perfLog) :
	jobManager{ jobManager },
	perfLog{ perfLog }
{
}

void PhysicsSystem::execute(World& world, float deltaTime, CollisionSystem& collSys)
{
	debugDrawables.clear();
	applyPhysics(deltaTime, collSys, world);
}

void PhysicsSystem::end()
{
}


void PhysicsSystem::findIslands(CollisionSystem& collSys, World& world) {
	Timer t3(perfLog.getInputRef("physicsprepare"));

	entityToIsland.clear();
	entityToIsland.resize(world.maxEntityIndex(), -1);

	islandSizes.clear();
	int islandCount = 0;
	for (auto collInfo : collSys.collisionInfos) {
		if (collInfo.indexA > world.maxEntityIndex() || collInfo.indexB > world.maxEntityIndex()) {
			throw new std::exception("illegal entity id. Probably heap corruption in the collision infos array");
		}
		int a = collInfo.indexA; int b = collInfo.indexB;
		if (entityToIsland.at(b) != -1) {
			if (islandSizes.at(entityToIsland.at(b)) < maxIslandSize) {
				if (entityToIsland.at(a) != -1)
					islandSizes.at(entityToIsland.at(a)) -= 1;
				islandSizes.at(entityToIsland.at(b)) += 1;
				entityToIsland.at(a) = entityToIsland.at(b);
			}
		}
		else if (entityToIsland.at(a) != -1) {
			if (islandSizes.at(entityToIsland.at(a)) < maxIslandSize) {
				if (entityToIsland.at(b) != -1)
					islandSizes.at(entityToIsland.at(b)) -= 1;
				islandSizes.at(entityToIsland.at(a)) += 1;
				entityToIsland.at(collInfo.indexB) = entityToIsland.at(a);
			}
		}
		else {
			entityToIsland.at(collInfo.indexA) = islandCount;
			entityToIsland.at(collInfo.indexB) = islandCount;
			islandCount++;
			islandSizes.push_back(2);
		}
	}
	for (int mergeIter = 0; mergeIter < islandMergeIterations; mergeIter++) {
		for (auto collInfo : collSys.collisionInfos) {
			int a = collInfo.indexA; int b = collInfo.indexB;
			if (entityToIsland.at(a) < entityToIsland.at(b) && entityToIsland.at(a) != -1 && entityToIsland.at(b) != -1) {
				if (islandSizes.at(entityToIsland.at(a)) < maxIslandSize) {
					islandSizes.at(entityToIsland.at(b)) -= 1;
					islandSizes.at(entityToIsland.at(a)) += 1;
					entityToIsland.at(b) = entityToIsland.at(a);
				}
			}
			else if (entityToIsland.at(b) < entityToIsland.at(a) && entityToIsland.at(b) != -1 && entityToIsland.at(a) != -1) {
				if (islandSizes.at(entityToIsland.at(b)) < maxIslandSize) {
					islandSizes.at(entityToIsland.at(a)) -= 1;
					islandSizes.at(entityToIsland.at(b)) += 1;
					entityToIsland.at(a) = entityToIsland.at(b);
				}
			}
		}
	}

	int borderCollisions = 0;
	int allCollisions = 0;
	for (auto collInfo : collSys.collisionInfos) {
		if (entityToIsland.at(collInfo.indexA) != entityToIsland.at(collInfo.indexB)) borderCollisions++;
		allCollisions++;
	}
	float islandToBorderRatio = (float)(allCollisions - borderCollisions) / (float)borderCollisions;

	if (enableAutomaticIslandMaxSize) {
		if (islandToBorderRatio < islandToBorderRatioTarget) {
			maxIslandSize = std::min(absoluteMaxIslandSize, maxIslandSize + 1);
		}
		else {
			maxIslandSize = std::max(absoluteMinIslandSize, maxIslandSize - 1);
		}
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
	for (auto ent : world.entity_view<PhysicsBody, Movement>()) {
		if (entityToIsland.at(ent) == -1) {

			world.getComp<Draw>(ent).color = Vec4(1,1,1,1);
		}
		else {
			int val = entityToIsland.at(ent) % 10;
			world.getComp<Draw>(ent).color = colors.at(val);
		}
	}

	for (auto collInfo : collSys.collisionInfos)
	{
		auto pos = world.getComp<Base>(collInfo.indexA).position;
		auto pos2 = world.getComp<Base>(collInfo.indexB).position;
		float dist = distance(pos, pos2);
		float rota = getRotation(pos2 - pos);
		if (entityToIsland.at( collInfo.indexA) == entityToIsland.at(collInfo.indexB)) {
			auto scale = Vec2(dist, 0.02f);
			Drawable d(0, pos + normalize(pos2-pos) * dist*0.5f,1, scale, Vec4(0,0,0,1),Form::Rectangle, RotaVec2(rota));
			debugDrawables.push_back(d);
		}
		else {
			auto scale = Vec2(dist, 0.04f);
			Drawable d(0, pos + normalize(pos2 - pos) * dist * 0.5f, 1, scale, Vec4(1, 1, 1, 1), Form::Rectangle, RotaVec2(rota));
			debugDrawables.push_back(d);
		}
	}
	printf("DEBUG PhysicsSystem Islands:\n");
	printf("current islandSize maximum: %i\n", maxIslandSize);
	printf("collisions inside of islands: %i, collisions between islands: %i, ratio: %f\n", allCollisions- borderCollisions, borderCollisions, islandToBorderRatio);
#else
#ifdef DEBUG_COLLISION_LINES
	for (auto collInfo : collSys.collisionInfos)
	{
		auto pos = world.getComp<Base>(collInfo.indexA).position;
		auto pos2 = world.getComp<Base>(collInfo.indexB).position;
		float dist = distance(pos, pos2);
		float rota = getRotation(pos2 - pos);
		auto scale = Vec2(dist, 0.04f);
		Drawable d(0, pos + normalize(pos2 - pos) * dist * 0.5f, 0.99f, scale, Vec4(1, 1, 1, 1), Form::Rectangle, RotaVec2(rota));
		debugDrawables.push_back(d);
	}
#endif
#endif
#ifdef DEBUG_COLLISION_POINTS
	for (auto collInfo : collSys.collisionInfos) {
		Vec2 scale = Vec2(0.06, 0.06);
		Drawable d(0, collInfo.collisionPos, 1, scale, Vec4(1, 0, 0, 1), Form::Circle, RotaVec2(0));
		debugDrawables.push_back(d);
	}
#endif // DEBUG_COLLISION_POINTS

	// build vectors of collision info islands:
	for (auto& batch : collInfoIslands) 
		batch.clear();
	collInfoIslands.resize(islandCount);
	collInfoIslandsBorder.clear();
	for (auto collInfo : collSys.collisionInfos) {
		bool test1 = entityToIsland.at(collInfo.indexA) == entityToIsland.at(collInfo.indexB);
		bool test2 = entityToIsland.at(collInfo.indexA) != -1;

		if (test1 && test2) {
			int index = entityToIsland.at(collInfo.indexA);
			collInfoIslands.at(index).push_back(collInfo);
		}
		else {
			collInfoIslandsBorder.push_back(collInfo);
		}
	}

	int currentIslandHead = 0;
	int currentBatchCollCount = collInfoIslands.at(0).size();

	islandBatches.clear();
	islandBatches.reserve(islandCount / impulseResolutionMaxBatchSize + 1);
	for (int island = 1; island < islandCount; ++island) {
		if ((currentBatchCollCount + collInfoIslands.at(island).size()) > impulseResolutionMaxBatchSize) {
			islandBatches.push_back(std::pair(currentIslandHead, island));
			currentIslandHead = island;
			currentBatchCollCount = collInfoIslands.at(island).size();
		}
		else {
			currentBatchCollCount += collInfoIslands.at(island).size();
		}
	}
	if (currentIslandHead != islandCount) {
		islandBatches.push_back(std::pair(currentIslandHead, islandCount));
	}

	// execute big badges first
	std::sort(islandBatches.begin(), islandBatches.end(), [&](std::pair<int,int> a, std::pair<int, int> b) {
		size_t sizeA = 0;
		for (int i = a.first; i < a.second; ++i) {
			sizeA += collInfoIslands[i].size();
		}
		size_t sizeB = 0;
		for (int i = b.first; i < b.second; ++i) {
			sizeB += collInfoIslands[i].size();
		}
		return sizeA > sizeB;
	});
}

void PhysicsSystem::applyPhysics(float deltaTime, CollisionSystem& collSys, World& world)
{

	if (!collSys.collisionInfos.empty()) {
		// generate collisions islands and island batches
		findIslands(collSys, world);
		Timer t3(perfLog.getInputRef("physicsimpulse"));

		// Pushout job buffer fill and start
		velocityBuffer.clear();
		velocityBuffer.resize(world.maxEntityIndex());
		overlapAccumBuffer.clear();
		overlapAccumBuffer.resize(world.maxEntityIndex());
		for (auto ent : world.entity_view<Movement, PhysicsBody>()) {
			velocityBuffer[ent] = world.getComp<Movement>(ent).velocity;
			overlapAccumBuffer[ent] = world.getComp<PhysicsBody>(ent).overlapAccum;
		}

		PushoutCalcJob pushOutCalcJob = PushoutCalcJob(collSys.getAllCollisions(), velocityBuffer, overlapAccumBuffer, collSys.poolWorkerData->collisionResponses, world);
		auto pushOutCalcJobTag = jobManager.addJob(&pushOutCalcJob);

		for (auto ent : world.entity_view<PhysicsBody>()) {
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
					auto& solidB = world.getComp<PhysicsBody>(entB);
					auto& baseB = world.getComp<Base>(entB);
					Movement dummy = Movement();
					Movement& moveB = (world.hasComp<Movement>(entB) ? world.getComp<Movement>(entB) : dummy);
					auto& moveA = world.hasComp<Movement>(entA) ? world.getComp<Movement>(entA) : dummy;

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

		// execute impulse resolution
		for (int i = 0; i < impulseResulutionIterations; i++) {
			resolutionJobs.clear(),
				resolutionJobs.reserve(collInfoIslands.size());
			resolutionJobTags.clear();
			resolutionJobTags.reserve(collInfoIslands.size());
			for (auto& batch : islandBatches) {				// start jobs for island solving
				resolutionJobs.push_back(ImpulseResolutionJob(world, deltaTime, collInfoIslands, batch));
				resolutionJobTags.push_back(jobManager.addJob(&resolutionJobs.back()));
			}
			jobManager.waitAndHelp(&resolutionJobTags);
			for (auto& collInfo : collInfoIslandsBorder) {	// compute the rest (borders wbbetween badges) sequential
				collisionResolution(collInfo);
			}
		}

		jobManager.waitFor(pushOutCalcJobTag);

		propagateChildPushoutToParent(collSys, world);	// propagate child overlap ppushout to childs
	}
	Timer t3(perfLog.getInputRef("physicsrest"));

	// let entities sleep or wake them up
	for (auto entity : world.entity_view<Movement, Collider, Base>()) {
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
	for (auto ent : world.entity_view<LinearEffector>()) {
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
	for (auto ent : world.entity_view<FrictionEffector>()) {
		auto& moveField = world.getComp<FrictionEffector>(ent);
		auto [begin, end] = collSys.getCollisions(ent);
		for (auto iter = begin; iter != end; ++iter) {
			if (world.hasComps<Movement, PhysicsBody>(iter->indexB)) {
				world.getComp<Movement>(iter->indexB).velocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).friction));
				world.getComp<Movement>(iter->indexB).angleVelocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).rotationalFriction));
			}
		}
	}

	for (auto ent : world.entity_view<PhysicsBody, Movement, Base>()) {
		auto& mov = world.getComp<Movement>(ent);
		auto& base = world.getComp<Base>(ent);
		// uniform effector execution :
		auto& solid = world.getComp<PhysicsBody>(ent);
		mov.velocity *= (1 / (1 + deltaTime * std::min(world.physics.friction, solid.friction)));
		mov.angleVelocity *= (1 / (1 + deltaTime * std::min(world.physics.friction, solid.friction)));
		mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectAccel * deltaTime;
		mov.velocity += world.physics.linearEffectDir * world.physics.linearEffectForce / solid.mass * deltaTime;
		// apply pushout:
		base.position += collSys.poolWorkerData->collisionResponses[ent].posChange;
	}

	syncBaseChildrenToParents(world);

	// submit debug drawables for physics
	for (auto& el : Physics::debugDrawables) {
		debugDrawables.push_back(el);
	}
#ifdef DEBUG_PRESSURE

	float allClipping = 0.0f;
	float maxClipping = 0.0f;
	for (auto ent : world.entity_view<PhysicsBody, Draw>()) {
		auto& phys = world.getComp<PhysicsBody>(ent);
		maxClipping = std::max(maxClipping, phys.overlapAccum);
		allClipping += phys.overlapAccum;
	}
	for (auto ent : world.entity_view<PhysicsBody, Draw>()) {
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
	for (auto ent : world.entity_view<Collider>()) {
		auto& collider = world.getComp<Collider>(ent);
		assert(!(collider.particle && collider.sleeping));
	}
#endif
}

void PhysicsSystem::propagateChildPushoutToParent(CollisionSystem& collSys, World& world)
{
	for (auto child : world.entity_view<BaseChild, PhysicsBody>()) {
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

void PhysicsSystem::syncBaseChildrenToParents(World& world) {
	for (auto child : world.entity_view<BaseChild>()) {
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
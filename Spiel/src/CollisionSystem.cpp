#pragma once
#include "CollisionSystem.hpp"

#include "PhysicsSystem.hpp"
#include "CacheAABBJob.hpp"

CollisionSystem::CollisionSystem(World& world, JobManager& jobManager, uint32_t qtreeCapacity) :
	world{ world },
	jobManager{ jobManager },
	qtreeCapacity{ qtreeCapacity },
	workerBuffers(jobManager.neededBufferNum()),
	qtreeDynamic({ 0,0 }, { 0,0 }, qtreeCapacity, world, jobManager, Collider::DYNAMIC),
	qtreeStatic({ 0,0 }, { 0,0 }, qtreeCapacity, world, jobManager, Collider::STATIC),
	qtreeParticle({ 0,0 }, { 0,0 }, qtreeCapacity, world, jobManager, Collider::PARTICLE),
	qtreeSensor({ 0,0 }, { 0,0 }, qtreeCapacity, world, jobManager, Collider::SENSOR)
{
	jobEntityBuffers.push_back(std::make_unique<std::vector<EntityHandleIndex>>());
}

void CollisionSystem::execute(World& world, float deltaTime)
{
	prepare(world);
	collisionDetection(world);
}

std::vector<CollisionInfo>& CollisionSystem::getCollisions()
{
	return this->collisionInfos;
}

const CollisionSystem::CollisionsView CollisionSystem::collisions_view(EntityHandleIndex entity)
{
	if (world.hasComp<CollisionsToken>(entity)) {
		return CollisionsView((size_t)world.getComp<CollisionsToken>(entity).begin, (size_t)world.getComp<CollisionsToken>(entity).end, collisionInfos);
	}
	else {
		return CollisionsView(collisionInfos.size(), collisionInfos.size(), collisionInfos);
	}
}

const std::vector<Drawable>& CollisionSystem::getDebugDrawables() const
{
	return debugDrawables;
}

void CollisionSystem::checkForCollisions(std::vector<CollisionInfo>& collisions, uint8_t colliderType, Transform const& b, Collider const& c) const
{
	Vec2 aabb = aabbBounds(c.size, b.rotaVec);
	std::vector<EntityHandleIndex> near;
	if (colliderType & Collider::DYNAMIC) {
		qtreeDynamic.querry(near, b.position, aabb);
	}
	if (colliderType & Collider::STATIC) {
		qtreeStatic.querry(near, b.position, aabb);
	}
	if (colliderType & Collider::PARTICLE) {
		qtreeParticle.querry(near, b.position, aabb);
	}
	if (colliderType & Collider::SENSOR) {
		qtreeSensor.querry(near, b.position, aabb);
	}
	std::vector<CollPoint> verteciesBuffer;
	generateCollisionInfos(world, collisions, aabbCache, near, 0xFFFFFFFF, b, c, aabb, verteciesBuffer);
}

void CollisionSystem::prepare(World& world)
{

	rebuildStatic = true;

	// allocate memory for collider groups and or clean them
	cleanBuffers(world);

	// TODO REDO SPLIT OF COLLIDER
	// split collidables
	Vec2 minPos{ 0,0 }, maxPos{ 0,0 };
	for (auto colliderEnt : world.entityView<Collider>()) {
		auto colliderID = colliderEnt.index;
		auto& collider = world.getComp<Collider>(colliderID);
		auto& baseCollider = world.getComp<Transform>(colliderID);
		minPos = min(minPos, baseCollider.position);
		maxPos = max(maxPos, baseCollider.position);
	
		if (world.hasComp<PhysicsBody>(colliderID)) { // if a collider has a solidBody, it is a physics object
			if (world.hasComp<Movement>(colliderID)) {	// is it dynamic or static?
				if (collider.particle) {
					particleEntities.push_back(colliderID);
				}
				else {
					dynamicSolidEntities.push_back(colliderID);
				}
			}
			else {	// entity must be static
				staticSolidEntities.push_back(colliderID);
			}
		}
		else { // if a collider has NO PhysicsBody, it is a sensor
			sensorEntities.push_back(colliderID);
		}
	}

	CacheAABBJob aabbJob0 = CacheAABBJob(particleEntities, world, aabbCache);
	auto jobTagAABBparticle = jobManager.addJob(&aabbJob0);

	CacheAABBJob aabbJob1 = CacheAABBJob(dynamicSolidEntities, world, aabbCache);
	auto jobTagAABBDyn = jobManager.addJob(&aabbJob1);

	CacheAABBJob aabbJob2 = CacheAABBJob(staticSolidEntities, world, aabbCache);
	auto jobTagAABBStat = jobManager.addJob(&aabbJob2);

	CacheAABBJob aabbJob3 = CacheAABBJob(sensorEntities, world, aabbCache);
	auto jobTagAABBSensor = jobManager.addJob(&aabbJob3);

	jobManager.waitFor(jobTagAABBparticle);
	jobManager.waitFor(jobTagAABBDyn);
	jobManager.waitFor(jobTagAABBStat);
	jobManager.waitFor(jobTagAABBSensor);

	// clean quadtrees
	qtreeParticle.resetPerMinMax(minPos, maxPos);
	qtreeParticle.removeEmptyLeafes();
	qtreeDynamic.resetPerMinMax(minPos, maxPos);
	qtreeDynamic.removeEmptyLeafes();
	//if (rebuildStatic) {
		qtreeStatic.resetPerMinMax(minPos, maxPos);
		qtreeStatic.removeEmptyLeafes();
	//}
	qtreeSensor.resetPerMinMax(minPos, maxPos);
	qtreeSensor.removeEmptyLeafes();

	qtreeParticle.broadInsert(particleEntities, aabbCache);
	qtreeDynamic.broadInsert(dynamicSolidEntities, aabbCache);
	//if (rebuildStatic) {
		qtreeStatic.broadInsert(staticSolidEntities, aabbCache);
	//}
	qtreeSensor.broadInsert(sensorEntities, aabbCache);
}

void CollisionSystem::cleanBuffers(World& world)
{
	auto cleanAndShrink = [this] (auto& vector) {
		if (vector.capacity() >= vector.size() * 50) {
			vector.shrink_to_fit();
		}
		vector.clear();
	};
	workerBuffers.clear();
	cleanAndShrink(debugDrawables);
	cleanAndShrink(particleEntities);
	cleanAndShrink(sensorEntities);
	cleanAndShrink(dynamicSolidEntities);
	cleanAndShrink(staticSolidEntities);
	cleanAndShrink(aabbCache);
	if (aabbCache.size() < world.maxEntityIndex()) aabbCache.resize(world.maxEntityIndex());
	cleanAndShrink(collisionInfos);
	for (auto ent : world.entityView<Collider>()) {
		if (!world.hasComp<CollisionsToken>(ent))
			world.addComp<CollisionsToken>(ent);
		else {
			world.getComp<CollisionsToken>(ent).begin = 0;
			world.getComp<CollisionsToken>(ent).end = 0;
		}
	}

	cleanAndShrink(collisionInfos);

	cleanAndShrink(collisionCheckJobs);
	cleanAndShrink(collisionCheckJobTags);

	for (auto& jobBuffer : jobEntityBuffers) {
		jobBuffer->clear();
	}
}

void CollisionSystem::collisionDetection(World& world)
{

	// the maximum job count is the size of the 4 entity vectors divided by the max job size (+1 for the rest of the division)
	// multiplied by the number of qtrees the entitiylist checks with
	// we MUST reserve the jobBUffer size here, as a reallocation would mean, that a running job would have a dangling ppointer to the old
	// memory of the job, after a reallocation of the job vector.
	const int maximumJobCount =
		(particleEntities.size() / MAX_ENTITIES_PER_JOB + 1) * 2 +
		(dynamicSolidEntities.size() / MAX_ENTITIES_PER_JOB + 1) * 2 +
		(staticSolidEntities.size() / MAX_ENTITIES_PER_JOB + 1) * 1 +
		(sensorEntities.size() / MAX_ENTITIES_PER_JOB + 1) * 4;
	collisionCheckJobs.reserve(maximumJobCount);

	size_t cap = collisionCheckJobs.capacity();

	int currentBuffer = 0;
	auto makeJobs = [&](const std::vector<EntityHandleIndex>& entities, const uint8_t qtreeMask) {
		auto pushJob = [&]() {
			if (collisionCheckJobs.size() == maximumJobCount - 1)
				throw new std::exception("ERROR: DO NOT REALLOCATE JOB VECTOR WHEN EXECUTING JOBS!");
			collisionCheckJobs.push_back(CollisionCheckJob(world, workerBuffers, *jobEntityBuffers[currentBuffer], qtreeDynamic, qtreeParticle, qtreeStatic, qtreeSensor, qtreeMask, aabbCache));
			collisionCheckJobTags.push_back(jobManager.addJob(&collisionCheckJobs.back()));
			++currentBuffer;
			if (currentBuffer >= jobEntityBuffers.size()) {
				jobEntityBuffers.push_back(std::make_unique<std::vector<EntityHandleIndex>>());
				jobEntityBuffers.back()->reserve(MAX_ENTITIES_PER_JOB);
			}
		};
		for (const auto ent : entities) {
			jobEntityBuffers[currentBuffer]->push_back(ent);
			if (jobEntityBuffers[currentBuffer]->size() == MAX_ENTITIES_PER_JOB) {
				pushJob();
			}
		}
		if (jobEntityBuffers[currentBuffer]->size() != 0) {
			pushJob();
		}
	};

	makeJobs(particleEntities, Collider::DYNAMIC | Collider::STATIC);
	
	makeJobs(dynamicSolidEntities, Collider::DYNAMIC | Collider::STATIC);

	makeJobs(staticSolidEntities, Collider::DYNAMIC);

	makeJobs(sensorEntities, Collider::PARTICLE | Collider::DYNAMIC | Collider::SENSOR | Collider::STATIC);

	if (collisionCheckJobs.capacity() != cap)
		throw new std::exception();

	jobManager.waitAndHelp(&collisionCheckJobTags);

	// reset quadtree rebuild flags
	rebuildStatic = false;

	// store all collisioninfos in one vector
	for (auto const& collisionInfosplit : workerBuffers.collisionInfos) {
		collisionInfos.insert(collisionInfos.end(), collisionInfosplit.cbegin(), collisionInfosplit.cend());
	}

	if (collisionInfos.size() > 0) {
		EntityHandleIndex currentEntity = collisionInfos[0].indexA;
		world.getComp<CollisionsToken>(currentEntity).begin = 0;
		for (int i = 1; i < collisionInfos.size(); i++) {
			if (currentEntity != collisionInfos[i].indexA) {	//new idA found
				auto nextEntity = collisionInfos[i].indexA;
				world.getComp<CollisionsToken>(nextEntity).begin = i;
				world.getComp<CollisionsToken>(currentEntity).end = i;
				currentEntity = nextEntity;	//set lastId to new id
			}
		}
		world.getComp<CollisionsToken>(currentEntity).end = collisionInfos.size();
	}

	for (EntityHandle ent : world.entityView<Movement, Collider>()) {
		Movement& mov = world.getComp<Movement>(ent);
		if (mov.velocity.length() < 0.00001f) {
			mov.velocity = Vec2(0, 0);
			world.getComp<Collider>(ent).sleeping = true;
		}
		else {
			world.getComp<Collider>(ent).sleeping = false;
		}
	}
#define DEBUG_SLEEP
#ifdef DEBUG_SLEEP
	for (auto ent : world.entityView<Collider>()) {
		auto& base = world.getComp<Transform>(ent);
		auto& coll = world.getComp<Collider>(ent);
		if (coll.sleeping)
			debugDrawables.push_back(Drawable(0, base.position, 0.81, Vec2(0.1,0.1), Vec4(0, 1, 0, 1), Form::Circle, RotaVec2(0)));
	}
#endif
#ifdef DEBUG_HITBOX
	for (auto ent : world.entity_view<Collider>()) {
		auto& base = world.getComp<Base>(ent);
		auto& coll = world.getComp<Collider>(ent);

		debugDrawables.push_back(Drawable(0, base.position, 0.41, coll.size, Vec4(0, 1, 0, 1), coll.form, base.rotaVec));
		for (auto& c : coll.extraColliders) {
			debugDrawables.push_back(Drawable(0, base.position + rotate(c.relativePos, base.rotaVec), 0.41, c.size, Vec4(0, 1, 0, 1), c.form, base.rotaVec * c.relativeRota));
		}
	}
#endif
#ifdef DEBUG_AABB
	for (auto ent : world.entity_view<Collider>()) {
		auto& base = world.getComp<Base>(ent);
		auto& coll = world.getComp<Collider>(ent);

		debugDrawables.push_back(Drawable(0, base.position, 0.4, aabbCache.at(ent), Vec4(1, 0, 0, 1), Form::Rectangle, RotaVec2(0.0f)));
	}
#endif
#ifdef DEBUG_QTREE_FINDPLAYER
	EntityHandleIndex player;
	for (auto p : world.entity_view<Player>())
		player = p;

	std::vector<EntityHandleIndex> near;
	size_t max = 0;
	for (auto ent : world.entity_view<Collider, Movement>()) {
		near.clear();
		qtreeDynamic.querry(near, world.getComp<Base>(ent).position, aabbCache.at(ent));
		for (auto oent : near) {
			if (oent == player) {
				auto pos = world.getComp<Base>(ent).position;
				auto d = Drawable(0, pos, 0.99, Vec2(0.3,0.3), Vec4(0, 0, 1, 1), Form::Circle, 0);
				debugDrawables.push_back(d);
			}
		}
		max = std::max(max, near.size());
	}
#endif
}
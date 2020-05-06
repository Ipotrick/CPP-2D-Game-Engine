#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem(World& world, uint32_t threadCount, PerfLogger& perfLog, float statCollGridRes, uint32_t qtreeCapacity) :
	CoreSystem( world ),
	threadCount{ threadCount },
	perfLog{ perfLog },
	qtreeCapacity{ qtreeCapacity }
{
	poolWorkerData = std::make_shared<PhysicsPoolData>(PhysicsPoolData(world, qtreeCapacity));
	poolWorkerData->staticCollisionGrid = GridPhysics<bool>(statCollGridRes);

	perWorkerData = std::vector<std::shared_ptr<PhysicsPerThreadData>>(threadCount);
	for (int id = 0; id < threadCount; id++) {
		perWorkerData[id] = std::make_shared<PhysicsPerThreadData>();
		perWorkerData[id]->id = id;
	}

	syncWorkerData = std::make_shared<PhysicsSharedSyncData>();
	syncWorkerData->go = std::vector<bool>(threadCount, false);
	for (unsigned i = 0; i < threadCount; i++) {
		workerThreads.push_back(std::thread(PhysicsWorker(perWorkerData.at(i), poolWorkerData, syncWorkerData, threadCount)));
		workerThreads.at(i).detach();
	}

	collisionInfosSplit.resize(threadCount);
}

void PhysicsSystem::execute(float deltaTime)
{
	debugDrawables.clear();
	poolWorkerData->debugDrawables.clear();
	prepare();
	collisionDetection();
	applyPhysics(deltaTime);
	debugDrawables.insert(debugDrawables.end(), poolWorkerData->debugDrawables.begin(), poolWorkerData->debugDrawables.end());
}

std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> PhysicsSystem::getCollisions(entity_handle entity)
{
	auto begin = collisionInfoBegins.find(entity);
	auto end = collisionInfoEnds.find(entity);
	if (begin != collisionInfoBegins.end() && end != collisionInfoEnds.end()) {	// is there even collisionInfo for the id?
		return std::make_tuple( begin->second, end->second );
	}
	else {
		return std::make_tuple( collisionInfos.end(), collisionInfos.end() );
	}
}

GridPhysics<bool> PhysicsSystem::getStaticGrid()
{
	return poolWorkerData->staticCollisionGrid;
}

void PhysicsSystem::end()
{
	syncWorkerData->run = false;
	std::unique_lock switch_lock(syncWorkerData->mut);
	for (unsigned i = 0; i < threadCount; i++) {
		syncWorkerData->go.at(i) = true;
	}
}

void PhysicsSystem::prepare()
{
	Timer t1(perfLog.getInputRef("physicsprepare"));

	poolWorkerData->rebuildDynQuadTrees = true; // allways rebuild dynamic quadtree
	poolWorkerData->rebuildStatQuadTrees = world.didStaticsChange(); // only rebuild static quadtree if static Entities changed

	// allocate memory for collider groups
	poolWorkerData->sensorCollidables.clear();
	poolWorkerData->sensorCollidables.reserve(world.getEntMemSize());
	poolWorkerData->dynCollidables.clear();
	poolWorkerData->dynCollidables.reserve(world.getEntMemSize());
	poolWorkerData->statCollidables.clear();
	poolWorkerData->statCollidables.reserve(world.getEntMemSize());

	Vec2 sensorMaxPos{ 0,0 }, sensorMinPos{ 0,0 };
	Vec2 dynMaxPos{ 0,0 }, dynMinPos{ 0,0 };
	Vec2 statMaxPos{ 0,0 }, statMinPos{ 0,0 };
	for (auto colliderID : world.view<Collider>()) {
		auto& collider = world.getComp<Collider>(colliderID);
		auto& baseCollider = world.getComp<Base>(colliderID);

		if (world.hasComp<PhysicsBody>(colliderID)) { // if a collider has a solidBody, it is a physics object
			if (world.hasComp<Movement>(colliderID)) {	// is it dynamic or static?
				poolWorkerData->dynCollidables.push_back(colliderID);
				dynMaxPos = max(dynMaxPos, baseCollider.position);
				dynMinPos = min(dynMinPos, baseCollider.position);
			}
			else {	// entity must be static
				poolWorkerData->statCollidables.push_back(colliderID);
				statMaxPos = max(statMaxPos, baseCollider.position);
				statMinPos = min(statMinPos, baseCollider.position);
			}
		}
		else { // if a collider has NO PhysicsBody, it is a sensor
			poolWorkerData->sensorCollidables.push_back(colliderID);
			sensorMaxPos = max(sensorMaxPos, baseCollider.position);
			sensorMinPos = min(sensorMinPos, baseCollider.position);
		}
	}

	// split the entities between threads 
	float splitStepSensor = (float)poolWorkerData->sensorCollidables.size() / (float)(threadCount);
	std::vector<std::array<int, 2>> rangesSensor(threadCount);
	for (unsigned i = 0; i < threadCount; i++) {
		rangesSensor[i][0] = static_cast<int>(floorf(i * splitStepSensor));
		rangesSensor[i][1] = static_cast<int>(floorf((i + 1) * splitStepSensor));
	}
	float splitStepDyn = (float)poolWorkerData->dynCollidables.size() / (float)(threadCount);
	std::vector<std::array<int, 2>> rangesDyn(threadCount);
	for (unsigned i = 0; i < threadCount; i++) {
		rangesDyn[i][0] = static_cast<int>(floorf(i * splitStepDyn));
		rangesDyn[i][1] = static_cast<int>(floorf((i + 1) * splitStepDyn));
	}
	float splitStepStat = (float)poolWorkerData->statCollidables.size() / (float)(threadCount);
	std::vector<std::array<int, 2>> rangesStat(threadCount);
	for (unsigned i = 0; i < threadCount; i++) {
		rangesStat[i][0] = static_cast<int>(floorf(i * splitStepStat));
		rangesStat[i][1] = static_cast<int>(floorf((i + 1) * splitStepStat));
	}

	poolWorkerData->collisionResponses.clear();
	if (poolWorkerData->collisionResponses.size() != world.getEntMemSize()) poolWorkerData->collisionResponses.resize(world.getEntMemSize());

	poolWorkerData->aabbCache.clear();
	if (poolWorkerData->aabbCache.size() != world.getEntMemSize()) poolWorkerData->aabbCache.resize(world.getEntMemSize());

	// give physics workers their info
	// write physics pool data
	if (poolWorkerData->rebuildDynQuadTrees) {
		poolWorkerData->qtreeDynamic.resetPerMinMax(dynMinPos, dynMaxPos);
		poolWorkerData->qtreeDynamic.removeEmptyLeafes();
	}
	if (poolWorkerData->rebuildStatQuadTrees) {
		poolWorkerData->qtreeStatic.resetPerMinMax(statMinPos, statMaxPos);
		poolWorkerData->qtreeStatic.removeEmptyLeafes();
	}
	// write physics individual data
	for (auto& split : collisionInfosSplit) split.clear();
	for (unsigned i = 0; i < threadCount; i++) {
		auto& pData = perWorkerData[i];
		pData->beginSensor = rangesSensor[i][0];
		pData->endSensor = rangesSensor[i][1];
		pData->beginDyn = rangesDyn[i][0];
		pData->endDyn = rangesDyn[i][1];
		pData->beginStat = rangesStat[i][0];
		pData->endStat = rangesStat[i][1];
		pData->collisionInfos = &collisionInfosSplit[i];
	}
	t1.stop();
}

void PhysicsSystem::collisionDetection()
{
	// check for collisions 
	Timer t2(perfLog.getInputRef("physicscollide"));
	{	// start physics threads
		std::unique_lock switch_lock(syncWorkerData->mut);
		for (unsigned i = 0; i < threadCount; i++) {
			syncWorkerData->go.at(i) = true;
		}
		syncWorkerData->cond.notify_all();
		// wait for physics threads to finish
		syncWorkerData->cond.wait(switch_lock, [&]() {
			/* wenn alle false sind wird true returned */
			for (unsigned i = 0; i < threadCount; i++) {
				if (syncWorkerData->go.at(i) == true) {
					return false;
				}
			}
			return true;
			}
		);
	}
	// reset quadtree rebuild flags6
	poolWorkerData->rebuildDynQuadTrees = false;
	poolWorkerData->rebuildStatQuadTrees = false;

	// store all collisioninfos in one vector
	collisionInfos.clear();
	collisionInfos.reserve(world.getEntMemSize()); // ~one collisioninfo per entity minumum capacity
	for (auto collisionInfosplit : collisionInfosSplit) {
		collisionInfos.insert(collisionInfos.end(), collisionInfosplit.begin(), collisionInfosplit.end());
	}

	// build hashtables for first and last iterator element of collisioninfo
	collisionInfoBegins.clear();
	collisionInfoEnds.clear();
	uint32_t lastIDA{};
	for (auto iter = collisionInfos.begin(); iter != collisionInfos.end(); ++iter) {
		if (iter == collisionInfos.begin()) {	//initialize values from first element
			lastIDA = iter->idA;
			collisionInfoBegins.insert({ iter->idA, iter });
		}
		if (lastIDA != iter->idA) {	//new idA found
			collisionInfoEnds.insert({ lastIDA, iter });
			collisionInfoBegins.insert({ iter->idA, iter });
			lastIDA = iter->idA;	//set lastId to new id
		}
	}
	collisionInfoEnds.insert({ lastIDA, collisionInfos.end() });

	t2.stop();
}

void PhysicsSystem::applyPhysics(float deltaTime)
{
	Timer t3(perfLog.getInputRef("physicsexecute"));

// collision info operations begin:
	// execute inelastic collisions 
	for (auto& collInfo : collisionInfos) {
		uint32_t entA = collInfo.idA;
		uint32_t entB = collInfo.idB;

		if (world.hasComp<PhysicsBody>(entA) & world.hasComp<PhysicsBody>(entB)) { //check if both are solid

			// owner stands in place for the slave for a collision response execution
			if (world.hasComp<BaseChild>(entA) | world.hasComp<BaseChild>(entB)) {
				if (world.hasComp<BaseChild>(entA) && !world.hasComp<BaseChild>(entB)) {
					entA = world.getEnt(world.getComp<BaseChild>(entA).parent);
				}
				else if (!world.hasComp<BaseChild>(entA) && world.hasComp<BaseChild>(entB)) {
					entB = world.getEnt(world.getComp<BaseChild>(entB).parent);
				}
				else {
					// both are slaves
					entA = world.getEnt(world.getComp<BaseChild>(entA).parent);
					entB = world.getEnt(world.getComp<BaseChild>(entB).parent);
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
					collInfo.collisionNormal, collInfo.collisionPos, elast,friction);
				moveA.velocity += collChanges.first;
				moveA.angleVelocity += collChanges.second;
				moveB.velocity += otherChanges.first;
				moveB.angleVelocity += otherChanges.second;

			}
		}
	}

	// let entities sleep or wake them up
	for (auto entity : world.view<Movement, Collider, Base>()) {
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
	for (auto ent : world.view<LinearEffector>()) {
		auto& moveField = world.getComp<LinearEffector>(ent);
		auto [begin, end] = getCollisions(ent);
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
	for (auto ent : world.view<FrictionEffector>()) {
		auto& moveField = world.getComp<FrictionEffector>(ent);
		auto [begin, end] = getCollisions(ent);
		for (auto iter = begin; iter != end; ++iter) {
			if (world.hasComps<Movement, PhysicsBody>(iter->idB)) {
				world.getComp<Movement>(iter->idB).velocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).friction));
				world.getComp<Movement>(iter->idB).angleVelocity *= (1 / (1 + deltaTime * world.getComp<FrictionEffector>(ent).rotationalFriction));
			}
		}
	}

	// uniform effector execution :
	for (auto ent : world.view<Movement, PhysicsBody>()) {
		auto& mov = world.getComp<Movement>(ent);
		auto& solid = world.getComp<PhysicsBody>(ent);
		mov.velocity *= (1 / (1 + deltaTime * std::min(world.uniformsPhysics.friction, solid.friction)));
		mov.angleVelocity *= (1 / (1 + deltaTime * std::min(world.uniformsPhysics.friction, solid.friction)));
		mov.velocity += world.uniformsPhysics.linearEffectDir * world.uniformsPhysics.linearEffectAccel * deltaTime;
		mov.velocity += world.uniformsPhysics.linearEffectDir * world.uniformsPhysics.linearEffectForce / solid.mass * deltaTime;
	}
// effector operations end!
// velocity and pushout operations:

	// apply dampened collision response pushout of slave to owner
	propagateChildPushoutToParent();

	// apply pushout
	for (auto ent : world.view<Movement, PhysicsBody, Base>()) {
		auto& base = world.getComp<Base>(ent);
		base.position += poolWorkerData->collisionResponses[ent].posChange;
	}

	// execute physics changes in pos, rota
	for (auto ent : world.view<Movement, Base>()) {

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
			draw.color = Vec4(0, 0, 0, 1);
		}
		else {
			draw.color = Vec4(0, 1, 1, 1);
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

void PhysicsSystem::propagateChildPushoutToParent()
{
	for (auto child : world.view<BaseChild, PhysicsBody>()) {
		auto relationship = world.getComp<BaseChild>(child);
		auto parent = world.getEnt(relationship.parent);
		float childWeight = norm(poolWorkerData->collisionResponses[child].posChange);
		float parentWeight = norm(poolWorkerData->collisionResponses[parent].posChange);
		float normalizer = childWeight + parentWeight;
		if (normalizer > Physics::nullDelta) {
			poolWorkerData->collisionResponses[parent].posChange = (childWeight * poolWorkerData->collisionResponses[child].posChange + parentWeight * poolWorkerData->collisionResponses[parent].posChange) / normalizer;
		}
	}
}

void PhysicsSystem::syncBaseChildrenToParents() {
	for (auto child : world.view<BaseChild>()) {
		auto& base = world.getComp<Base>(child);
		auto& movement = world.getComp<Movement>(child);
		auto& relationship = world.getComp<BaseChild>(child);

		auto parent = world.getEnt(relationship.parent);
		auto& baseP = world.getComp<Base>(parent);
		auto& movementP = world.getComp<Movement>(parent);

		base.position = baseP.position + rotate(relationship.relativePos, baseP.rotation);
		movement.velocity = movementP.velocity;
		base.rotation = baseP.rotation + relationship.relativeRota;
		movement.angleVelocity = movementP.angleVelocity;
	}
}
#include "Engine.h"

#define NOMINMAX
#include <windows.h>

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 100 },// 10000 microseconds = 10 milliseond => 100 loops per second
	deltaTime{ 0.0 },
	mainTime{ 0.0 },
	updateTime{ 0.0 },
	physicsTime{ 0.0 },	// TODO REFACTOR ALL TIME DATA INTO STRUCT
	physicsPrepareTime{ 0.0 },
	physicsCollisionTime{ 0.0 },
	physicsExecuteTime{ 0.0 },
	renderBufferPushTime{ 0.0 },
	mainSyncTime{ 0.0 },
	mainWaitTime{ 0.0 },
	renderTime{ 0.0 },
	new_deltaTime{ 0 },
	new_mainTime{ 0 },
	new_updateTime{ 0 },
	new_physicsTime{ 0 },
	new_physicsPrepareTime{ 0 },
	new_physicsCollisionTime{ 0 },
	new_physicsExecuteTime{ 0 },
	new_renderTime{ 0 },
	new_renderBufferPushTime{ 0 },
	new_mainSyncTime{ 0 },
	new_mainWaitTime{ 0 },
	collInfos{},
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_)},
	renderer{ window },
	windowSpaceDrawables{},
	physicsThreadCount{ std::thread::hardware_concurrency() - 1 },
	qtreeCapacity{ 7 },
	staticGrid{ {0.2f, 0.2f} },
	rebuildStaticData{ true }
{
	physicsPerThreadData = std::vector<std::shared_ptr<PhysicsPerThreadData>>(physicsThreadCount);
	physicsPoolData = std::make_shared<PhysicsPoolData>(PhysicsPoolData());
	physicsPoolData->world = &world;
	int n = 0;
	for (auto& el : physicsPerThreadData) {
		el = std::make_shared<PhysicsPerThreadData>();
		el->id = n++;
	}
	sharedPhysicsSyncData = std::make_shared<PhysicsSharedSyncData>();
	sharedPhysicsSyncData->go = std::vector<bool>(physicsThreadCount, false);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		physicsThreads.push_back(std::thread(PhysicsWorker(physicsPerThreadData.at(i), physicsPoolData, sharedPhysicsSyncData, physicsThreadCount)));
		SetThreadPriority(physicsThreads.back().native_handle(), 15);
		physicsThreads.at(i).detach();
	}
}

Engine::~Engine() {
	renderer.end();
	{
		std::lock_guard<std::mutex> l(sharedPhysicsSyncData->mut);
		sharedPhysicsSyncData->run = false;
	}
}

std::string Engine::getPerfInfo(int detail)
{
	std::stringstream ss;
	if (detail >= 4) ss << "Entities: " << world.getEntCount() << "\n";
	ss << "deltaTime(s): " << deltaTime << " ticks/s: " << (1 / deltaTime) << " simspeed: " << getDeltaTimeSafe()/ deltaTime << '\n';
	if (detail >= 1) ss << "    mainTime(s): "   << mainTime << " mainSyncTime(s): " << mainSyncTime << " mainWaitTime(s): " << mainWaitTime <<'\n';
	if (detail >= 2) ss << "        update(s): " << updateTime    << " physics(s): " << physicsTime << " renderBufferPush(s): " << renderBufferPushTime << " staticGridBuildTime: " << staticGridBuildTime << '\n';
	if (detail >= 3) ss << "            physicsPrepare(s): " << physicsPrepareTime << " physicsCollisionTime(s): " << physicsCollisionTime << " physicsExecuteTime(s): " << physicsExecuteTime << '\n';
	if (detail >= 1) ss << "    renderTime(s): " << renderTime << " renderSyncTime(s): " << renderSyncTime << '\n';

	return ss.str();
}

InputStatus Engine::getKeyStatus(KEY key_)
{
	std::lock_guard<std::mutex> l(window->mut);
	return (InputStatus)glfwGetKey(window->glfwWindow, int(key_));
}

bool Engine::keyPressed(KEY key_)
{
	return getKeyStatus(key_) == InputStatus::PRESS;
}

bool Engine::keyReleased(KEY key_)
{
	return getKeyStatus(key_) == InputStatus::RELEASE;
}

bool Engine::keyRepeating(KEY key_)
{
	return getKeyStatus(key_) == InputStatus::REPEAT;
}

vec2 Engine::getCursorPos()
{
	vec2 size = getWindowSize();
	std::lock_guard<std::mutex> l(window->mut);
	double xPos, yPos;
	glfwGetCursorPos(window->glfwWindow, &xPos, &yPos);
	return { (float)xPos / size.x * 2.0f - 1.f, -(float)yPos / size.y * 2.0f +1.f };
}

InputStatus Engine::getButtonStatus(BUTTON but_)
{
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<InputStatus>( glfwGetMouseButton(window->glfwWindow, static_cast<int>(but_)));
}

bool Engine::buttonPressed(BUTTON but_)
{
	return getButtonStatus(but_) == InputStatus::PRESS;
}

bool Engine::buttonReleased(BUTTON but_)
{
	return getButtonStatus(but_) == InputStatus::RELEASE;
}

vec2 Engine::getWindowSize()
{
	std::lock_guard<std::mutex> l(window->mut);
	return { static_cast<float>(window->width), static_cast<float>(window->height) };
}

float Engine::getWindowAspectRatio()
{
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<float>(window->width)/ static_cast<float>(window->height);
}

vec2 Engine::getPosWorldSpace(vec2 windowSpacePos_)
{
	auto transformedPos = mat4::translate(camera.position) * mat4::rotate_z(camera.rotation) * mat4::scale(vec2(1 / camera.frustumBend.x, 1/ camera.frustumBend.y)) * mat4::scale(1/camera.zoom) * vec4(windowSpacePos_.x, windowSpacePos_.y, 0, 1);
	return { transformedPos.x, transformedPos.y };
}

std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> Engine::getCollisionInfos(uint32_t id_)
{
	auto begin = collInfoBegins.find(id_);
	auto end = collInfoEnds.find(id_);
	if (begin != collInfoBegins.end() && end != collInfoEnds.end()) {	// is there even collisionInfo for the id?
		return { begin->second, end->second };
	}
	else {
		return { collInfos.end(), collInfos.end() };
	}
}

void Engine::commitTimeMessurements() {
	deltaTime = micsecToFloat(new_deltaTime);
	mainTime = micsecToFloat(new_mainTime);
	updateTime = micsecToFloat(new_updateTime);
	physicsTime = micsecToFloat(new_physicsTime);
	physicsPrepareTime = micsecToFloat(new_physicsPrepareTime);
	physicsCollisionTime = micsecToFloat(new_physicsCollisionTime);
	physicsExecuteTime = micsecToFloat(new_physicsExecuteTime);
	staticGridBuildTime = micsecToFloat(new_staticGridBuildTime);
	renderTime = micsecToFloat(new_renderTime);
	mainSyncTime = micsecToFloat(new_mainSyncTime);
	mainWaitTime = micsecToFloat(new_mainWaitTime);
	renderBufferPushTime = micsecToFloat(new_renderBufferPushTime);
	renderSyncTime = micsecToFloat(new_renderSyncTime);
}

void Engine::run() {
	create();

	while (running) {
		Timer loopTimer(new_deltaTime);
		Waiter<> loopWaiter(minimunLoopTime, Waiter<>::Type::BUSY, &new_mainWaitTime);
		glfwPollEvents();
		commitTimeMessurements();
		rebuildStaticData = false; // reset flag
		{
			Timer mainTimer(new_mainTime);
			renderer.startRendering();
			{
				Timer t(new_updateTime);
				update(world, getDeltaTimeSafe());
				world.slaveOwnerDespawn();
				world.deregisterDespawnedEntities();
				world.executeDespawns();
				if (world.didStaticsChange()) rebuildStaticData = true;
			}
			{
				Timer t(new_physicsTime);
				physicsUpdate(world, getDeltaTimeSafe());
			}
			{
				Timer t(new_staticGridBuildTime);
				updateStaticGrid(world);
			}
			{
				Timer t(new_renderBufferPushTime);
				rendererUpdate(world);
			}
		}
		if (glfwWindowShouldClose(window->glfwWindow)) { // if window closes the program ends
			running = false;
		}
		iteration++;
	}

	destroy();
}

// TODO REFACTOR INTO SUB FUNCTIONS
// TODO MAYBE seperate physics and collision detection
void Engine::physicsUpdate(World& world_, float deltaTime_)
{
	Timer<> t1(new_physicsPrepareTime);

	physicsPoolData->rebuildDynQuadTrees = true; // allways rebuild dynamic quadtree
	physicsPoolData->rebuildStatQuadTrees = rebuildStaticData; // only rebuild static quadtree if static Entities changed

	// allocate memory for colliders
	std::vector<uint32_t> dynCollidables;
	dynCollidables.reserve(world.entities.size());
	std::vector<uint32_t> statCollidables;
	statCollidables.reserve(world.entities.size());

	std::vector<CollisionResponse> collisionResponses(world.entities.size());
	vec2 dynMaxPos{ 0,0 }, dynMinPos{ 0,0 };
	vec2 statMaxPos{ 0,0 }, statMinPos{ 0,0 };
	for (auto iter = world.getAll<Collider>().begin(); iter != world.getAll<Collider>().end(); ++iter) {
		auto& collider = *iter;
		auto colliderID = iter.id();
		auto& baseCollider = world.getComp<Base>(colliderID);

		if (world.hasComp<Movement>(colliderID) || collider.dynamic) {
			dynCollidables.push_back(colliderID);
			dynMaxPos.x = std::max(dynMaxPos.x, baseCollider.position.x);
			dynMaxPos.y = std::max(dynMaxPos.y, baseCollider.position.y);
			dynMinPos.x = std::min(dynMinPos.x, baseCollider.position.x);
			dynMinPos.y = std::min(dynMinPos.y, baseCollider.position.y);
		}
		else if (world.hasComp<SolidBody>(colliderID)) {
			statCollidables.push_back(colliderID);
			statMaxPos.x = std::max(statMaxPos.x, baseCollider.position.x);
			statMaxPos.y = std::max(statMaxPos.y, baseCollider.position.y);
			statMinPos.x = std::min(statMinPos.x, baseCollider.position.x);
			statMinPos.y = std::min(statMinPos.y, baseCollider.position.y);
		}
	}
	
	// split the entities between threads 
	float splitStepDyn = (float)dynCollidables.size() / (float)(physicsThreadCount);
	std::vector<std::array<int ,2>> rangesDyn(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		rangesDyn[i][0] = static_cast<int>(floorf(i * splitStepDyn));
		rangesDyn[i][1] = static_cast<int>(floorf((i + 1) * splitStepDyn));
	}
	float splitStepStat = (float)statCollidables.size() / (float)(physicsThreadCount);
	std::vector<std::array<int, 2>> rangesStat(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		rangesStat[i][0] = static_cast<int>(floorf(i * splitStepStat));
		rangesStat[i][1] = static_cast<int>(floorf((i + 1) * splitStepStat));
	}

	// give physics workers their info
	// write physics pool data
	physicsPoolData->dynCollidables = &dynCollidables;
	physicsPoolData->statCollidables = &statCollidables;
	physicsPoolData->collisionResponses = &collisionResponses;
	physicsPoolData->world = &world;
	if (physicsPoolData->rebuildDynQuadTrees) {
		// make new quadtrees and replace old ones
		std::shared_ptr<std::vector<Quadtree>> qtreesDynamic;
		qtreesDynamic = std::make_shared< std::vector<Quadtree>>();
		qtreesDynamic->reserve(physicsThreadCount);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			qtreesDynamic->emplace_back(Quadtree(dynMinPos, dynMaxPos, qtreeCapacity));
		}
		physicsPoolData->qtreesDynamic = qtreesDynamic; // replace dynamic quadtrees
	}
	if (physicsPoolData->rebuildStatQuadTrees) { 
		// make new quadtrees and replace old ones
		std::shared_ptr<std::vector<Quadtree>> qtreesStatic;
		qtreesStatic = std::make_shared< std::vector<Quadtree>>();
		qtreesStatic->reserve(physicsThreadCount);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			qtreesStatic->emplace_back(Quadtree(statMinPos, statMaxPos, qtreeCapacity));
		}
		physicsPoolData->qtreesStatic = qtreesStatic; // replace static quadtrees
	}
	// write physics individual data
	std::vector<std::vector<CollisionInfo>> collisionInfosSplit(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		auto& pData = physicsPerThreadData[i];
		pData->beginDyn = rangesDyn[i][0];
		pData->endDyn = rangesDyn[i][1];
		pData->beginStat = rangesStat[i][0];
		pData->endStat = rangesStat[i][1];
		pData->collisionInfos = &collisionInfosSplit[i];
	}
	t1.stop();
	/* check for collisions */
	Timer<> t2(new_physicsCollisionTime);

	{	// start physics threads
		std::unique_lock switch_lock(sharedPhysicsSyncData->mut);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			sharedPhysicsSyncData->go.at(i) = true;
		}
		sharedPhysicsSyncData->cond.notify_all();
		// wait for physics threads to finish
		sharedPhysicsSyncData->cond.wait(switch_lock, [&]() { 
			/* wenn alle false sind wird true returned */
			for (unsigned i = 0; i < physicsThreadCount; i++) {
				if (sharedPhysicsSyncData->go.at(i) == true) {
					return false;
				}
			}
			return true; 
			}
		);
	}
	// reset quadtree rebuild flags
	physicsPoolData->rebuildDynQuadTrees = false;
	physicsPoolData->rebuildStatQuadTrees = false;

	t2.stop();
	// execute physics
	Timer<> t3(new_physicsExecuteTime);
	//store all collisioninfos in one vector
	collInfos.clear();
	collInfos.reserve(world_.entities.size()); // ~one collisioninfo per entity minumum capacity
	for (auto collInfoSplit : collisionInfosSplit) {
		collInfos.insert(collInfos.end(), collInfoSplit.begin(), collInfoSplit.end());
	}

	// build hashtables for first and last iterator element of collisioninfo
	collInfoBegins.clear();
	collInfoEnds.clear();
	uint32_t lastIDA{};
	for (auto iter = collInfos.begin(); iter != collInfos.end(); ++iter) {
		if (iter == collInfos.begin()) {	//initialize values from first element
			lastIDA = iter->idA;
			collInfoBegins.insert({ iter->idA, iter });
		}
		if (lastIDA != iter->idA) {	//new idA found
			collInfoEnds.insert({ lastIDA, iter });
			collInfoBegins.insert({ iter->idA, iter });
			lastIDA = iter->idA;	//set lastId to new id
		}
	}
	collInfoEnds.insert({ lastIDA, collInfos.end() });

	// execute inelastic collisions 
	for (auto& collInfo : collInfos) {
		uint32_t entA = collInfo.idA;
		uint32_t entB = collInfo.idB;

		if (world.hasComp<SolidBody>(entA) & world.hasComp<SolidBody>(entB)) { //check if both are solid

			// owner stands in place for the slave for a collision response execution
			if (world.hasComp<Slave>(entA) | world.hasComp<Slave>(entB)) {
				if (world.hasComp<Slave>(entA) && !world.hasComp<Slave>(entB)) {
					entA = world.getComp<Slave>(entA).ownerID;
				}
				else if (!world.hasComp<Slave>(entA) && world.hasComp<Slave>(entB)) {
					entB = world.getComp<Slave>(entB).ownerID;
				}
				else {
					// both are slaves
					entA = world.getComp<Slave>(entA).ownerID;
					entB = world.getComp<Slave>(entB).ownerID;
				}
			}

			if (world.hasComp<SolidBody>(entA) & world.hasComp<SolidBody>(entB)) { // recheck if the owners are solid
				auto& solidA = world.getComp<SolidBody>(entA);
				auto& baseA = world.getComp<Base>(entA);
				auto& moveA = world.getComp<Movement>(entA);
				auto& solidB = world.getComp<SolidBody>(entB);
				auto& baseB = world.getComp<Base>(entB);
				Movement dummy = Movement();
				Movement& moveB = (world.hasComp<Movement>(entB) ? world.getComp<Movement>(entB) : dummy);

				auto& collidB = world.getComp<Collider>(entB);

				float elast = std::max(solidA.elasticity, solidB.elasticity);
				auto [collChanges, otherChanges] = dynamicCollision2d5(
					baseA.position, moveA.velocity, moveA.angleVelocity, solidA.mass, solidA.momentOfInertia,
					baseB.position, moveB.velocity, moveB.angleVelocity, solidB.mass, solidB.momentOfInertia,
					collInfo.collisionNormal, collInfo.collisionPos, elast);
				moveA.velocity += collChanges.first;
				moveA.angleVelocity += collChanges.second;
				if (collidB.dynamic) {
					moveB.velocity += otherChanges.first;
					moveB.angleVelocity += otherChanges.second;
				}
			}
		}
	}

	// apply dampened collision response pushout of slave to owner
	for (auto slaveEnt : world.view<Slave, Movement>()) {
		auto slave = world.getComp<Slave>(slaveEnt);
		auto slaveMov = world.getComp<Movement>(slaveEnt);
		float slaveWeight = norm(collisionResponses[slaveEnt].posChange);
		float ownerWeight = norm(collisionResponses[slave.ownerID].posChange);
		float normalizer = slaveWeight + ownerWeight;
		if (normalizer > Physics::nullDelta) {
			collisionResponses[slave.ownerID].posChange = (slaveWeight * collisionResponses[slaveEnt].posChange + ownerWeight * collisionResponses[slave.ownerID].posChange) / normalizer;
		}
	}

	// execute physics changes in pos, rota
	for (auto ent : world.view<Movement, Base>()) {
		auto& move = world.getComp<Movement>(ent);
		auto& base = world.getComp<Base>(ent);
		base.position += collisionResponses[ent].posChange;
		base.position += move.velocity * deltaTime_;
		base.rotation += move.angleVelocity * deltaTime_;
	}

	syncCompositPhysics<4>();
	t3.stop();

	// submit debug drawables for physics
	for (auto& el : Physics::debugDrawables) {
		submitDrawableWorldSpace(el);
	}
	Physics::debugDrawables.clear();
}

void Engine::updateStaticGrid(World& world)
{
	if (rebuildStaticData) {
		vec2 staticGridMinPos = vec2(0,0);
		vec2 staticGridMaxPos = vec2(0, 0);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			auto treeMin = physicsPoolData->qtreesStatic->at(i).getPosition() - physicsPoolData->qtreesStatic->at(i).getSize() * 0.5f;
			auto treeMax = physicsPoolData->qtreesStatic->at(i).getPosition() + physicsPoolData->qtreesStatic->at(i).getSize() * 0.5f;
			if (treeMin.x < staticGridMinPos.x) staticGridMinPos.x = treeMin.x;
			if (treeMin.y < staticGridMinPos.y) staticGridMinPos.y = treeMin.y;
			if (treeMax.x > staticGridMaxPos.x) staticGridMaxPos.x = treeMax.x;
			if (treeMax.y > staticGridMaxPos.y) staticGridMaxPos.y = treeMax.y;
		}
		staticGrid.minPos = staticGridMinPos;
		int xSize = static_cast<int>(ceilf((staticGridMaxPos.x - staticGridMinPos.x)) / staticGrid.cellSize.x);
		int ySize = static_cast<int>(ceilf((staticGridMaxPos.y - staticGridMinPos.y)) / staticGrid.cellSize.y);
		staticGrid.clear();
		staticGrid.resize(xSize, ySize);

		std::vector<uint32_t> nearCollidables;
		nearCollidables.reserve(20);
		for (int x = 0; x < staticGrid.getSizeX(); x++) {
			for (int y = 0; y < staticGrid.getSizeY(); y++) {
				vec2 pos = staticGrid.minPos + vec2(x, y) * staticGrid.cellSize;
				vec2 size = staticGrid.cellSize;
				CollidableAdapter collAdapter = CollidableAdapter(pos, 0, 0, size, Form::RECTANGLE, true);

				for (unsigned i = 0; i < physicsThreadCount; i++) {
					physicsPoolData->qtreesStatic->at(i).querry(nearCollidables, pos, size);
				}

				for (auto& otherID : nearCollidables) {
					vec2 velOther = (world.hasComp<Movement>(otherID) ? world.getComp<Movement>(otherID).velocity : vec2(0, 0));
					CollidableAdapter otherAdapter = CollidableAdapter(world.getComp<Base>(otherID).position, world.getComp<Base>(otherID).rotation, velOther, world.getComp<Collider>(otherID).size, world.getComp<Collider>(otherID).form, world.getComp<Collider>(otherID).dynamic);
					auto result = checkForCollision(&collAdapter, &otherAdapter, false);
					if (result.collided) {
						staticGrid.set(x, y, true);
						break;
					}
				}
				nearCollidables.clear();
			}
		}
	}
#ifdef DEBUG_STATIC_GRID
	Drawable d = Drawable(0, staticGrid.minPos, 0.1f, staticGrid.cellSize, vec4(1, 1, 0, 1), Form::RECTANGLE, 0.0f);
	for (int i = 0; i < staticGrid.getSizeX(); i++) {
		for (int j = 0; j < staticGrid.getSizeY(); j++) {
			d.position = staticGrid.minPos + vec2(i,0) * staticGrid.cellSize.x + vec2(0,j) * staticGrid.cellSize.y;
			if (staticGrid.at(i,j)) {
				submitDrawableWorldSpace(d);
			}
		}
	}
#endif
}

Drawable&& buildDrawable(World& world, ent_id_t entity) {
	return std::move(Drawable(entity, world.getComp<Base>(entity).position, world.getComp<Draw>(entity).drawingPrio, world.getComp<Draw>(entity).scale, world.getComp<Draw>(entity).color, world.getComp<Draw>(entity).form, world.getComp<Base>(entity).rotation, world.getComp<Draw>(entity).throwsShadow));
}

void Engine::rendererUpdate(World& world)
{
	renderer.waitTillFinished();
	for (auto ent : world.view<Base,Draw>()) {
		renderer.submit(buildDrawable(world, ent));
	}
	for (auto d : worldSpaceDrawables) {
		renderer.submit(d);
	}
	worldSpaceDrawables.clear();
	for (auto winD : windowSpaceDrawables) {
		renderer.submitWindowSpace(winD);
	}
	windowSpaceDrawables.clear();
	renderer.setCamera(camera);

	new_renderTime = renderer.getRenderingTime();
	new_renderSyncTime = renderer.getwaitedTime();
}

template<int N>
void Engine::syncCompositPhysics()
{
	auto & view = world.getAll<Composit<N>>();
	for (auto iter = view.begin(); iter != view.end(); ++iter) {
		auto& baseOwner = world.getComp<Base>(iter.id());
		auto& movOwner = world.getComp<Movement>(iter.id());

		for (int i = 0; i < N; i++) {
			if (iter->slaves[i].id != 0) {
				auto& baseSlave = world.getComp<Base>(iter->slaves[i].id);
				auto& movSlave = world.getComp<Movement>(iter->slaves[i].id);
				auto slaveComposidData = iter->slaves[i];
				baseSlave.position = baseOwner.position + rotate(slaveComposidData.relativePos, baseOwner.rotation);
				movSlave.velocity = movOwner.velocity;
				baseSlave.rotation = baseOwner.rotation + slaveComposidData.relativeRota;
				movSlave.angleVelocity = movOwner.angleVelocity;
			}
			else {
				break;
			}
		}
	}
}

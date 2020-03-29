#include "Engine.h"

#define NOMINMAX
#include <windows.h>

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 100 },// 10000 microseconds = 10 milliseond => 100 loops per second
	oldWorldEntitiesCapacity{ 0 },
	deltaTime{ 0.0 },
	mainTime{ 0.0 },
	updateTime{ 0.0 },
	physicsTime{ 0.0 },
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
	sharedRenderData{ std::make_shared<RendererSharedData>() },
	renderBufferA{},
	windowSpaceDrawables{},
	physicsThreadCount{ std::thread::hardware_concurrency() - 1},
	qtreeCapacity{ 5 },
	staticGrid{}
{
	window->initialize();
	renderThread = std::thread(Renderer(sharedRenderData, window));
	SetThreadPriority(renderThread.native_handle(), 0);
	renderThread.detach();
	windowSpaceDrawables.reserve(50);

	physicsPerThreadData = std::vector<std::shared_ptr<PhysicsPerThreadData>>(physicsThreadCount);
	physicsPoolData = std::make_shared<PhysicsPoolData>(PhysicsPoolData());
	physicsPoolData->world = &world;
	int n = 0;
	for (auto& el : physicsPerThreadData) {
		el = std::make_shared<PhysicsPerThreadData>();
		el->id = n++;
	}
	sharedPhysicsSyncData = std::make_shared<PhysicsSharedSyncData>();
	sharedPhysicsSyncData->go = std::vector<bool>(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		physicsThreads.push_back(std::thread(PhysicsWorker(physicsPerThreadData.at(i), physicsPoolData, sharedPhysicsSyncData, physicsThreadCount)));
		SetThreadPriority(physicsThreads.back().native_handle(), 15);
		physicsThreads.at(i).detach();
	}
}

Engine::~Engine() {
	{
		std::lock_guard<std::mutex> l(sharedRenderData->mut);
		sharedRenderData->run = false;
	}
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
		Timer<> loopTimer(new_deltaTime);
		Waiter<> loopWaiter(minimunLoopTime, Waiter<>::Type::BUSY, &new_mainWaitTime);
		commitTimeMessurements();
		glfwPollEvents();
		sharedRenderData->cond.notify_one();	// wake up rendering thread
		{
			Timer<> mainTimer(new_mainTime);
			{
				Timer<> t(new_updateTime);
				update(world, getDeltaTimeSafe());
				world.slaveOwnerDespawn();
				world.deregisterDespawnedEntities();
				world.executeDespawns();
			}
			if (world.entities.capacity() != oldWorldEntitiesCapacity || world.staticSpawnOrDespawn) {
				world.staticSpawnOrDespawn = false; // reset flag
				oldWorldEntitiesCapacity = world.entities.capacity();
				// when entities reallocate, the physics need to rebuild the quadtrees
				physicsPoolData->rebuildDynQuadTrees = true;
				physicsPoolData->rebuildStatQuadTrees = true;
				rebuildStaticGrid = true;
			}
			{
				Timer<> t(new_physicsTime);
				physicsUpdate(world, getDeltaTimeSafe());
			}
			{
				Timer<> t(new_staticGridBuildTime);
				updateStaticGrid(world);
			}
			{
				Timer<> t(new_renderBufferPushTime);
				renderBufferA.camera = Camera();
				renderBufferA.windowSpaceDrawables.clear();
				renderBufferA.worldSpaceDrawables.clear();
				
				for (auto& d : windowSpaceDrawables) renderBufferA.windowSpaceDrawables.push_back(d);
				auto puffer = world.getDrawableVec();
				renderBufferA.worldSpaceDrawables.insert(renderBufferA.worldSpaceDrawables.end(), puffer.begin(), puffer.end());
				for (auto& d : worldSpaceDrawables) renderBufferA.worldSpaceDrawables.push_back(d);
				renderBufferA.camera = camera;
			
				windowSpaceDrawables.clear();
				worldSpaceDrawables.clear();
			}
		}
		
		{	
			Timer<> t(new_mainSyncTime);
			std::unique_lock<std::mutex> switch_lock(sharedRenderData->mut);
			sharedRenderData->cond.wait(switch_lock, [&]() { return sharedRenderData->ready == true; });	// wait for rendering thread to finish
			sharedRenderData->ready = false;																// reset renderers ready flag
			sharedRenderData->renderBufferB = renderBufferA;												// push Drawables and camera
			new_renderTime = sharedRenderData->new_renderTime;	// save render time
			new_renderSyncTime = sharedRenderData->new_renderSyncTime;
			// light data
			sharedRenderData->lightCollisions.clear();
			for (auto& light : world.lightCompCtrl.componentData) {
				auto [begin, end] = getCollisionInfos(light.first);
				for (auto iter = begin; iter != end; ++iter) {
					sharedRenderData->lightCollisions.push_back(*iter);
				}
			}
			sharedRenderData->lights = world.getLightVec();

			if (sharedRenderData->run == false) {
				running = false;
			}
		}

		iteration++;
	}

	destroy();
}

void Engine::physicsUpdate(World& world_, float deltaTime_)
{
	Timer<> t1(new_physicsPrepareTime);

	physicsPoolData->rebuildDynQuadTrees = true; // allways rebuild dynamic quadtree
	// only rebuild static quadtree if either new static entitites are spawned/ removed/ moved or the world entity vector reallocated

	std::vector<std::pair<uint32_t, Collidable *>> dynCollidables;
	dynCollidables.reserve(world.entities.size());
	std::vector<std::pair<uint32_t, Collidable*>> statCollidables;
	statCollidables.reserve(world.entities.size());
	vec2 maxPosDynamic{ 0,0 }, minPosDynamic{ 0,0 };
	vec2 maxPosStatic{ 0,0 }, minPosStatic{ 0,0 };
	for (int id = 1; id < world.getEntMemSize(); id++) {
		if (world.doesEntExist(id)) {
			auto& el = world.getEntity(id);
			if (el.isDynamic()) {
				dynCollidables.push_back({ id, (Collidable*) & (el) });	// build dynamic collidable vector
				if (el.position.x < minPosDynamic.x) minPosDynamic.x = el.position.x;
				if (el.position.y < minPosDynamic.y) minPosDynamic.y = el.position.y;
				if (el.position.x > maxPosDynamic.x) maxPosDynamic.x = el.position.x;
				if (el.position.y > maxPosDynamic.y) maxPosDynamic.y = el.position.y;
			}
			else {
				statCollidables.push_back({ id, (Collidable*) & (el) });	// build static collidable vector
				if (el.position.x < minPosStatic.x) minPosStatic.x = el.position.x;
				if (el.position.y < minPosStatic.y) minPosStatic.y = el.position.y;
				if (el.position.x > maxPosStatic.x) maxPosStatic.x = el.position.x;
				if (el.position.y > maxPosStatic.y) maxPosStatic.y = el.position.y;
			}
		}
	}
	
	// rebuild dynamic quadtrees
	std::shared_ptr<std::vector<Quadtree>> qtreesDynamic;
	if (physicsPoolData->rebuildDynQuadTrees) {
		qtreesDynamic = std::make_shared< std::vector<Quadtree>>();
		qtreesDynamic->reserve(physicsThreadCount);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			vec2 randOffsetMin = { +(rand() % 1000 / 2000.0f) + 1, +(rand() % 1000 / 2000.0f) + 1 };
			vec2 randOffsetMax = { +(rand() % 1000 / 2000.0f) + 1, +(rand() % 1000 / 2000.0f) + 1 };
			qtreesDynamic->emplace_back(Quadtree(minPosDynamic - randOffsetMin, maxPosDynamic + randOffsetMax, qtreeCapacity));
		}
	}
	// rebuild static quadtrees
	std::shared_ptr<std::vector<Quadtree>> qtreesStatic;
	if (physicsPoolData->rebuildStatQuadTrees) {
		qtreesStatic = std::make_shared< std::vector<Quadtree>>();
		qtreesStatic->reserve(physicsThreadCount);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			vec2 randOffsetMin = { +(rand() % 1000 / 2000.0f) + 1, +(rand() % 1000 / 2000.0f) + 1 };
			vec2 randOffsetMax = { +(rand() % 1000 / 2000.0f) + 1, +(rand() % 1000 / 2000.0f) + 1 };
			qtreesStatic->emplace_back(Quadtree(minPosStatic - randOffsetMin, maxPosStatic + randOffsetMax, qtreeCapacity));
		}
	}

	std::vector<CollisionResponse> collisionResponses(world.entities.size());
	t1.stop();

	/* check for collisions */
	Timer<> t2(new_physicsCollisionTime);
	/* split the entities between threads */
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
	std::vector<std::vector<CollisionInfo>> collisionInfosSplit(physicsThreadCount);
	physicsPoolData->dynCollidables = &dynCollidables;
	physicsPoolData->statCollidables = &statCollidables;
	physicsPoolData->collisionResponses = &collisionResponses;
	physicsPoolData->world = &world;
	if (physicsPoolData->rebuildDynQuadTrees) physicsPoolData->qtreesDynamic = qtreesDynamic; // reassign dynamic quadtrees
	if (physicsPoolData->rebuildStatQuadTrees) physicsPoolData->qtreesStatic = qtreesStatic; // reassign static quadtrees
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		auto& pData = physicsPerThreadData[i];
		pData->beginDyn = rangesDyn[i][0];
		pData->endDyn = rangesDyn[i][1];
		pData->beginStat = rangesStat[i][0];
		pData->endStat = rangesStat[i][1];
		pData->collisionInfos = &collisionInfosSplit[i];
	}

	{	// start physics threads
		std::unique_lock switch_lock(sharedPhysicsSyncData->mut);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			sharedPhysicsSyncData->go.at(i) = true;
		}
		sharedPhysicsSyncData->cond.notify_all();
		//wait for physics threads to finish
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
	physicsPoolData->rebuildDynQuadTrees = false;
	physicsPoolData->rebuildStatQuadTrees = false;
	t2.stop();

	Timer<> t3(new_physicsExecuteTime);
	//store all collisioninfos in one vector
	collInfos.clear();
	collInfos.reserve(world_.entities.size()); // ~one collisioninfo per entity minumum capacity
	for (auto collInfoSplit : collisionInfosSplit) {
		collInfos.insert(collInfos.end(), collInfoSplit.begin(), collInfoSplit.end());
	}

	//build hastable for first and last iterator element of collisioninfo
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
		auto* coll = world.getEntityPtr(collInfo.idA);
		auto* other = world.getEntityPtr(collInfo.idB);

		if (coll->isSolid() & other->isSolid()) {
			SolidBody* solidColl = world.getCompPtr<SolidBody>(collInfo.idA);
			SolidBody* solidOther = world.solidBodyCompCtrl.getComponentPtr(collInfo.idB);
			// owner stands in place for the slave for a collision response execution
			if (coll->isSlave() | other->isSlave()) {
				if (coll->isSlave() && !other->isSlave()) {
					solidColl = world.solidBodyCompCtrl.getComponentPtr(coll->getOwnerID());
					coll = world.getEntityPtr(coll->getOwnerID());
					assert(coll);
					assert(solidColl);
				}
				else if (!coll->isSlave() && other->isSlave()) {
					solidOther = world.solidBodyCompCtrl.getComponentPtr(other->getOwnerID());
					other = world.getEntityPtr(other->getOwnerID());
					assert(other);
					assert(solidOther);
				}
				else {
					// both are slaves
					solidColl = world.solidBodyCompCtrl.getComponentPtr(coll->getOwnerID());
					coll = world.getEntityPtr(coll->getOwnerID());
					solidOther = world.solidBodyCompCtrl.getComponentPtr(other->getOwnerID());
					other = world.getEntityPtr(other->getOwnerID());
					assert(coll);
					assert(solidColl);
					assert(other);
					assert(solidOther);
				}
			}
			
			float elast = std::max(solidColl->elasticity, solidOther->elasticity);
			auto [collChanges, otherChanges] = dynamicCollision2d4(*coll, solidColl->mass, solidColl->momentOfInertia, *other, solidOther->mass, solidOther->momentOfInertia, collInfo.collisionNormal, collInfo.collisionPos, elast);
			coll->velocity += collChanges.first;
			coll->angleVelocity += collChanges.second;
			if (other->isDynamic()) {
				other->velocity += otherChanges.first;
				other->angleVelocity += otherChanges.second;
			}
		}
	}

	//apply dampened pushout of slave to owner
	for (auto& coll : dynCollidables) {
		//pushout owner too
		if (coll.second->isSlave() && coll.second->isSolid()) {
			uint32_t ownerID = coll.second->getOwnerID();

			float slaveWeight = norm(collisionResponses[coll.first].posChange);
			float ownerWeight = norm(collisionResponses[ownerID].posChange);
			float normalizer = slaveWeight + ownerWeight;
			if (normalizer > Physics::nullDelta) {
				collisionResponses[ownerID].posChange = (slaveWeight * collisionResponses[coll.first].posChange + ownerWeight * collisionResponses[ownerID].posChange) / normalizer;
			}
		}
	}

	// execute physics changes in pos, rota
	for (auto& coll : dynCollidables) {
		coll.second->position += collisionResponses[coll.first].posChange;	//non solids have a pushout of 0
		coll.second->position += coll.second->velocity * deltaTime_;
		coll.second->rotation += coll.second->angleVelocity * deltaTime;
	}

	syncCompositPhysics<4>(world.composit4CompCtrl);
	t3.stop();

	/* submit debug drawables for physics */
	for (auto& el : Physics::debugDrawables) {
		submitDrawableWorldSpace(el);
	}
	Physics::debugDrawables.clear();
}

void Engine::updateStaticGrid(World& world)
{
	if (rebuildStaticGrid) {
		rebuildStaticGrid = false;
		vec2 staticGridMinPos = vec2(0,0);
		vec2 staticGridMaxPos = vec2(0, 0);
		for (int i = 0; i < physicsThreadCount; i++) {
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

		std::vector<std::pair<uint32_t, Collidable*>> nearCollidables;
		for (int x = 0; x < staticGrid.getSizeX(); x++) {
			for (int y = 0; y < staticGrid.getSizeY(); y++) {
				vec2 pos = staticGrid.minPos + vec2(x, y) * staticGrid.cellSize;
				vec2 size = staticGrid.cellSize;
				auto tester = Collidable(size, Form::RECTANGLE, true);
				tester.position = pos;

				for (int i = 0; i < physicsThreadCount; i++) {
					physicsPoolData->qtreesStatic->at(i).querry(nearCollidables, pos, size);
				}

				for (auto& other : nearCollidables) {
					if (other.second->isSolid() && !other.second->isDynamic()) {
						auto result = checkForCollision(&tester, other.second, false);
						if (result.collided) {
							staticGrid.set(x, y, true);
							break;
						}
					}
				}
				nearCollidables.clear();
			}
		}
	}
	Drawable d = Drawable(0, staticGrid.minPos, 0.1f, staticGrid.cellSize, vec4(1, 1, 1, 1), Form::RECTANGLE, 0.0f);
	for (int i = 0; i < staticGrid.getSizeX(); i++) {
		for (int j = 0; j < staticGrid.getSizeY(); j++) {
			d.position = staticGrid.minPos + vec2(i,0) * staticGrid.cellSize.x + vec2(0,j) * staticGrid.cellSize.y;
			if (staticGrid.at(i,j)) {
				submitDrawableWorldSpace(d);
			}
		}
	}
}

template<int N>
void Engine::syncCompositPhysics(CompController<Composit<N>> & composit)
{
	for (auto iter = composit.componentData.begin(); iter != composit.componentData.end(); ++iter) {
		auto& owner = world.getEntity(iter->first);

		for (int i = 0; i < N; i++) {
			if (iter->second.slaves[i].id != 0) {
				auto slave = world.getEntityPtr(iter->second.slaves[i].id);
				auto slaveComposidData = &iter->second.slaves[i];
				slave->position = owner.getPos() + rotate(slaveComposidData->relativePos, owner.getRota());
				slave->velocity = owner.getVel();
				slave->rotation = owner.getRota() + slaveComposidData->relativeRota;
				slave->angleVelocity = owner.getAnglVel();
			}
			else {
				break;
			}
		}
	}
}

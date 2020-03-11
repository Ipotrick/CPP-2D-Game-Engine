#include "Engine.h"

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 40 },//10000 microseconds = 10 milliseond => 100 loops per second
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
	collisionInfos{},
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_)},
	sharedRenderData{ std::make_shared<RendererSharedData>() },
	renderBufferA{},
	windowSpaceDrawables{},
	physicsThreadCount{ 6 },
	qtreeCapacity{ 50 }
{
	window->initialize();
	renderThread = std::thread(Renderer(sharedRenderData, window));
	renderThread.detach();
	windowSpaceDrawables.reserve(50);

	sharedPhysicsData = std::vector<std::shared_ptr<PhysicsSharedData>>(physicsThreadCount);
	int n = 0;
	for (auto& el : sharedPhysicsData) {
		el = std::make_shared<PhysicsSharedData>();
		el->id = n++;
	}
	sharedPhysicsSyncData = std::make_shared<PhysicsSyncData>();
	sharedPhysicsSyncData->go = std::vector<bool>(physicsThreadCount);
	for (int i = 0; i < physicsThreadCount; i++) {
		physicsThreads.push_back(std::thread(PhysicsWorker(sharedPhysicsData.at(i), sharedPhysicsSyncData)));
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
	ss << "deltaTime(s): " << deltaTime << " ticks/s: " << (1 / deltaTime) << " simspeed: " << getDeltaTimeSafe()/ deltaTime << '\n';
	if (detail >= 1) ss << "    mainTime(s): "   << mainTime << " mainSyncTime(s): " << mainSyncTime << " mainWaitTime(s): " << mainWaitTime <<'\n';
	if (detail >= 2) ss << "        update(s): " << updateTime    << " physics(s): " << physicsTime << " renderBufferPush(s): " << renderBufferPushTime << '\n';
	if (detail >= 3) ss << "            physicsPrepare(s): " << physicsPrepareTime << " physicsCollisionTime(s): " << physicsCollisionTime << " physicsExecuteTime(s): " << physicsExecuteTime << '\n';
	if (detail >= 1) ss << "    renderTime(s): " << renderTime << " renderSyncTime(s): " << renderSyncTime << '\n';

	return ss.str();
}

KEYSTATUS Engine::getKeyStatus(KEY key_)
{
	std::lock_guard<std::mutex> l(window->mut);
	return (KEYSTATUS)glfwGetKey(window->glfwWindow, int(key_));
}

bool Engine::keyPressed(KEY key_)
{
	return getKeyStatus(key_) == KEYSTATUS::PRESS;
}

bool Engine::keyReleased(KEY key_)
{
	return getKeyStatus(key_) == KEYSTATUS::RELEASE;
}

bool Engine::keyRepeating(KEY key_)
{
	return getKeyStatus(key_) == KEYSTATUS::REPEAT;
}

vec2 Engine::getWindowSize()
{
	std::lock_guard<std::mutex> l(window->mut);
	int width, height;
	glfwGetWindowSize(window->glfwWindow, &width, &height);
	return { static_cast<float>(width), static_cast<float>(height) };
}

float Engine::getWindowAspectRatio()
{
	std::lock_guard<std::mutex> l(window->mut);
	int width, height;
	glfwGetWindowSize(window->glfwWindow, &width, &height);
	return static_cast<float>(width)/ static_cast<float>(height);
}

std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> Engine::getCollisionInfos(uint32_t id_) {
	/* !!!die collision infos mÅEsen geprdent sein, so dass alle idA's einer ent hintereinanderstehen!!! */
	auto begin = std::lower_bound(collisionInfos.begin(), collisionInfos.end(), id_,
		[](CollisionInfo const& collInfo_, uint32_t id__) {
			return collInfo_.idA < id__;
		}
	);

	if (begin != collisionInfos.end() && begin->idA == id_) {
		auto end = std::next(begin);
		while (end != collisionInfos.end() && end->idA == id_) {
			end++;
		}
		return { begin, end };
	}
	else {
		return { collisionInfos.end(), collisionInfos.end() };
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
		sharedRenderData->cond.notify_one();	//wake up rendering thread

		{
			Timer<> mainTimer(new_mainTime);
			{
				Timer<> t(new_updateTime);
				update(world, getDeltaTime());
				world.executeDespawns();
			}
			{
				Timer<> t(new_physicsTime);
				physicsUpdate(world, getDeltaTimeSafe());
			}
			{
				Timer<> t(new_renderBufferPushTime);
				renderBufferA.camera = Camera();
				renderBufferA.windowSpaceDrawables.clear();
				renderBufferA.worldSpaceDrawables.clear();
				
				for (auto& d : windowSpaceDrawables) renderBufferA.windowSpaceDrawables.push_back(d);
				for (auto& ent : world.entities) renderBufferA.worldSpaceDrawables.push_back(ent);
				for (auto& d : worldSpaceDrawables) renderBufferA.worldSpaceDrawables.push_back(d);
				renderBufferA.camera = camera;
			
				windowSpaceDrawables.clear();
				worldSpaceDrawables.clear();
			}
		}
		
		{	
			Timer<> t(new_mainSyncTime);
			std::unique_lock<std::mutex> switch_lock(sharedRenderData->mut);
			sharedRenderData->cond.wait(switch_lock, [&]() { return sharedRenderData->ready == true; });	//wait for rendering thread to finish
			sharedRenderData->ready = false;																//reset renderers ready flag
			sharedRenderData->renderBufferB = renderBufferA;												//push Drawables and camera
			new_renderTime = sharedRenderData->new_renderTime;	//save render time
			new_renderSyncTime = sharedRenderData->new_renderSyncTime;
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
	collisionInfos.clear();
	collisionInfos.reserve(world_.entities.size()); //~one collisioninfo per entity minumum capacity

	std::vector<Collidable*> dynCollidables;
	vec2 maxPos, minPos;
	if (world_.entities.size() > 0) {
		maxPos = world_.entities.at(0).getPos();
		minPos = maxPos;
	}
	for (auto& el : world_.entities) {
		if (el.isDynamic()) {
			dynCollidables.push_back(&el);							//build dynamic collidable vector
		}
		//look if the quadtree has to take uop largera area
		if (el.position.x < minPos.x) minPos.x = el.position.x;
		if (el.position.y < minPos.y) minPos.y = el.position.y;
		if (el.position.x > maxPos.x) maxPos.x = el.position.x;
		if (el.position.y > maxPos.y) maxPos.y = el.position.y;
	}
	Quadtree qtree(minPos - vec2(1, 1), maxPos + vec2(1, 1), qtreeCapacity);
	for (auto& el : world_.entities) {
		qtree.insert(&el);
	}
	std::vector<CollisionResponse> collisionResponses(dynCollidables.size());
	t1.stop();

	/* check for collisions */
	Timer<> t2(new_physicsCollisionTime);
	/* split the entities between threads */
	float splitStep = (float)dynCollidables.size() / (float)(physicsThreadCount);
	std::vector<std::array<int ,2>> ranges(physicsThreadCount);
	for (int i = 0; i < physicsThreadCount; i++) {
		ranges[i][0] = floorf(i * splitStep);
		ranges[i][1] = floorf((i + 1) * splitStep);
	}
	/* submit debug drawables for physics */
	for (auto& el : Physics::debugDrawables) {
		submitDrawableWorldSpace(el);
	}
	Physics::debugDrawables.clear();

	//give physics workers their info
	std::vector<std::vector<CollisionInfo>> collisionInfosSplit(physicsThreadCount);
	for (int i = 0; i < physicsThreadCount; i++) {
		auto& pData = sharedPhysicsData[i];
		pData->begin = ranges[i][0];
		pData->end = ranges[i][1];
		pData->collisionInfos = &collisionInfosSplit[i];
		pData->collisionResponses = &collisionResponses;
		pData->qtree = &qtree;
		pData->dynCollidables = &dynCollidables;
		pData->deltaTime = deltaTime_;
	}

	{	// start physics threads
		std::unique_lock switch_lock(sharedPhysicsSyncData->mut);
		for (int i = 0; i < physicsThreadCount; i++) {
			sharedPhysicsSyncData->go.at(i) = true;
		}
		sharedPhysicsSyncData->cond.notify_all();
		//wait for physics threads to finish
		sharedPhysicsSyncData->cond.wait(switch_lock, [&]() { 
			/* wenn alle false sind wird true returned */
			for (int i = 0; i < physicsThreadCount; i++) {
				if (sharedPhysicsSyncData->go.at(i) == true) {
					return false;
				}
			}
			return true; 
			}
		);
	}
	t2.stop();

	//store all collisioninfos in one vector
	Timer<> t3(new_physicsExecuteTime);
	for (int i = 0; i < physicsThreadCount; i++) {
		for (auto& collinfo : (collisionInfosSplit[i])) {
			collisionInfos.push_back(collinfo);
		}
	}
	//execute physics changes in pos, vel, accel
	for (int i = 0; i < dynCollidables.size(); i++) {
		auto& coll = dynCollidables.at(i);
		coll->velocity += collisionResponses.at(i).velChange;
		coll->position += collisionResponses.at(i).posChange + coll->velocity * deltaTime_;
		coll->collided = collisionResponses.at(i).collided;
		coll->acceleration = 0;
	}
	t3.stop();
}
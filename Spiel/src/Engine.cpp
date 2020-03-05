#include "Engine.h"

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 1'00 },//10000 microseconds = 10 milliseond => 100 loops per second
	deltaTime{ 0.0 },
	mainTime{ 0.0 },
	updateTime{ 0.0 },
	physicsTime{ 0.0 },
	renderBufferPushTime{ 0.0 },
	mainSyncTime{ 0.0 },
	mainWaitTime{ 0.0 },
	renderTime{ 0.0 },
	new_deltaTime{ 0 },
	new_mainTime{ 0 },
	new_updateTime{ 0 },
	new_physicsTime{ 0 },
	new_renderTime{ 0 },
	new_renderBufferPushTime{ 0 },
	new_mainSyncTime{ 0 },
	new_mainWaitTime{ 0 },
	collisionInfos{},
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_)},
	sharedRenderData{ std::make_shared<RendererSharedData>() },
	renderBufferA{},
	windowSpaceDrawables{}
{
	window->initialize();
	renderThread = std::thread(Renderer(sharedRenderData, window));
	renderThread.detach();
	windowSpaceDrawables.reserve(50);
}

Engine::~Engine() {
	{
		std::lock_guard<std::mutex> l(sharedRenderData->mut);
		sharedRenderData->run = false;
	}
}

std::string Engine::getPerfInfo(int detail)
{
	std::stringstream ss;
	ss << "deltaTime(s): " << getDeltaTime() << " ticks/s: " << (1 / getDeltaTime()) << '\n';
	if (detail >= 1) ss << "    mainTime(s): "   << getMainTime() << " mainSyncTime(s): " << getMainSyncTime() << " mainWaitTime(s): " << mainWaitTime <<'\n';
	if (detail >= 2) ss << "        update(s): " << getUpdateTime()    << " physics(s): " << getPhysicsTime() << " renderBufferPush(s): " << renderBufferPushTime << '\n';
	if (detail >= 1) ss << "    renderTime(s): " << getRenderTime() << " renderSyncTime(s): " << renderSyncTime << '\n';


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

void Engine::commitTimeMessurements() {
	deltaTime = micsecToDouble(new_deltaTime);
	mainTime = micsecToDouble(new_mainTime);
	updateTime = micsecToDouble(new_updateTime);
	physicsTime = micsecToDouble(new_physicsTime);
	renderTime = micsecToDouble(new_renderTime);
	mainSyncTime = micsecToDouble(new_mainSyncTime);
	mainWaitTime = micsecToDouble(new_mainWaitTime);
	renderBufferPushTime = micsecToDouble(new_renderBufferPushTime);
	renderSyncTime = micsecToDouble(new_renderSyncTime);
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
				physicsUpdate(world, getDeltaTime());
			}
			{
				Timer<> t(new_renderBufferPushTime);
				renderBufferA.camera = Camera();
				renderBufferA.windowSpaceDrawables.clear();
				renderBufferA.worldSpaceDrawables.clear();
				
				for (auto& d : windowSpaceDrawables) renderBufferA.windowSpaceDrawables.push_back(d);
				for (auto& d : worldSpaceDrawables) renderBufferA.worldSpaceDrawables.push_back(d);
				for (auto& ent : world.entities) renderBufferA.worldSpaceDrawables.push_back(ent);
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

void Engine::physicsUpdate(World& world_, double deltaTime_)
{
	collisionInfos.clear();
	collisionInfos.reserve(world_.entities.size()); //~one collisioninfo per entity minumum capacity

	std::vector<Collidable*> dynCollidables;
	vec2 maxPos, minPos;
	if (world_.entities.size() > 0) {
		maxPos = world_.entities.at(0).getPos();
		minPos = maxPos;
	}
	for (auto & el : world_.entities) {
		if (el.isDynamic()) {
			dynCollidables.push_back(&el);							//build dynamic collidable vector
			el.position = el.position + el.velocity * deltaTime_;	//move collidable 
		}
		if (el.position.x < minPos.x) minPos.x = el.position.x;
		if (el.position.y < minPos.y) minPos.y = el.position.y;
		if (el.position.x > maxPos.x) maxPos.x = el.position.x;
		if (el.position.y > maxPos.y) maxPos.y = el.position.y;
	}
	Quadtree qtree(minPos-vec2(1,1), maxPos+vec2(1,1), 400);
	for (auto& el : world_.entities) {
		qtree.insert(&el);
	}
	/* DEBUG */
	submitDrawableWorldSpace(Drawable(qtree.getPosition(), 0, qtree.getSize(), vec4(1.0, 0.5, 0.5, 1), 0));

	std::vector<CollisionResponse> collisionResponses(dynCollidables.size());

	/* check for collisions */
	std::vector<Collidable*> nearCollidables;	//reuse heap memory for all dyn collidable collisions
	nearCollidables.reserve(dynCollidables.size());
	for (int i = 0; i < dynCollidables.size(); i++) {
		auto& coll = dynCollidables[i];
		nearCollidables.clear();
		qtree.querry(nearCollidables, coll->getPos(), coll->getBoundsSize());

		for (auto& other : nearCollidables) {
			auto newResponse = checkForCollision(coll, other);
			collisionResponses[i] = collisionResponses[i] + newResponse;
			if (newResponse.collided) {
				collisionInfos.emplace_back(CollisionInfo(coll->getId(), other->getId()));
			}
		}

	}

	for (int i = 0; i < dynCollidables.size(); i++) {
		auto& coll = dynCollidables.at(i);
		coll->velocity += collisionResponses[i].velChange;
		coll->position += collisionResponses[i].posChange + coll->velocity * deltaTime_;
		coll->collided = collisionResponses[i].collided;
	}
}

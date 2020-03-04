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
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_)},
	sharedRenderData{ std::make_shared<RendererSharedData>() },
	renderBufferA{}
{
	window->initialize();
	renderThread = std::thread(Renderer(sharedRenderData, window));
	renderThread.detach();
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
			}
			{
				Timer<> t(new_physicsTime);
				physicsUpdate(world, getDeltaTime());
			}
			{
				Timer<> t(new_renderBufferPushTime);
				renderBufferA.writeBuffer(world.getDrawableVec(), camera);
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

void Engine::physicsUpdate(World& world, double deltaTime)
{

}

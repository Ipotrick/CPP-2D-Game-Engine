#include "Engine.h"

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 100'0000 },//10000 microseconds = 10 milliseond => 100 loops per second
	deltaTime{ 0.0 },
	updateTime{ 0.0 },
	physicsTime{ 0.0 },
	renderTime{ 0.0 },
	new_deltaTime{ 0 },
	new_updateTime{ 0 },
	new_physicsTime{ 0 },
	new_renderTime{ 0 },
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_) },
	sharedRenderData{ std::make_shared<RendererSharedData>() },
	renderThread{ Renderer(window, sharedRenderData) },
	renderBufferA{}
{
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
	ss << "deltaTime(s): " << getDeltaTime() << " ticks/s: " << (1 / getDeltaTime()) << ' ';
	if (detail > 0) {
		ss << "renderTime: " << getRenderTime();
	}
	return ss.str();
}

void Engine::commitTimeMessurements() {
	deltaTime = micsecToDouble(new_deltaTime);
	updateTime = micsecToDouble(new_updateTime);
	physicsTime = micsecToDouble(new_physicsTime);
	renderTime = micsecToDouble(new_renderTime);

	new_deltaTime = std::chrono::microseconds(0);
	new_updateTime = std::chrono::microseconds(0);
	new_physicsTime = std::chrono::microseconds(0);
	new_renderTime = std::chrono::microseconds(0);
}

void Engine::run() {
	create();
	while (running && !glfwWindowShouldClose(window->glWindow)) {
		Timer<> loopTimer(new_deltaTime);
		Waiter<> loopWaiter(minimunLoopTime);
		commitTimeMessurements();
		glfwPollEvents();
		sharedRenderData->cond.notify_one();	//wake up rendering thread

		{
			Timer<> t(new_updateTime);
			update(world, getDeltaTime());
		}

		{
			Timer<> t(new_physicsTime);
			physicsUpdate(world, getDeltaTime());
		}

		renderBufferA.writeBuffer(world.getDrawableVec(), camera);
		{	
			std::unique_lock<std::mutex> switch_lock(sharedRenderData->mut);
			sharedRenderData->cond.wait(switch_lock, [&]() { return sharedRenderData->ready == true; });	//wait for rendering thread to finish
			sharedRenderData->ready = false;																//reset renderers ready flag
			sharedRenderData->renderBufferB = renderBufferA;												//push Drawables and camera
			new_renderTime = sharedRenderData->new_renderTime;												//save render time
		}

		iteration++;
	}

	destroy();
}

void Engine::physicsUpdate(World world, double deltaTime)
{

}

#include "Renderer.h"

#include "windows.h"

Renderer::Renderer(std::shared_ptr<Window> wndw) :
	window{ wndw },
	workerSharedData{ std::make_shared<RenderingSharedData>() },
	workerThread{ RenderingWorker(window, workerSharedData) },
	renderingTime{ 0 },
	syncTime{ 0 }
{
	SetThreadPriority(workerThread.native_handle(), 0);	// windows sceduling function
	workerThread.detach();
}

void Renderer::waitTillFinished()
{	
	Timer t(syncTime);
	std::unique_lock<std::mutex> switch_lock(workerSharedData->mut);
	workerSharedData->cond.wait(switch_lock, [&]() { return workerSharedData->ready == true; });	// wait for worker to finish
	// reset data
	workerSharedData->ready = false; // reset ready flag
	workerSharedData->renderBuffer.worldSpaceDrawables.clear();
	workerSharedData->renderBuffer.windowSpaceDrawables.clear();
	renderingTime = workerSharedData->new_renderTime;
}

void Renderer::startRendering()
{
	workerSharedData->cond.notify_one(); // wake up worker
}

void Renderer::end()
{
	waitTillFinished();
	workerSharedData->run = false;
	startRendering();
}

#include "Renderer.hpp"

using namespace std::literals::chrono_literals;

void Renderer::initialize(Window* wndw)
{
	assertIsState(RenderState::Uninitialized);
	state = RenderState::PreWait;

	window = wndw;
	workerSharedData = std::make_shared<RenderingSharedData>();
	workerThread = std::thread(RenderingWorker(window, workerSharedData));
	renderingTime = 0ms;
	syncTime = 0ms;
	frontBuffer = std::make_shared<RenderBuffer>();

	workerThread.detach();
	setLayerCount(1);
}

void Renderer::waitTillFinished() {	
	assertIsState(RenderState::PreWait);
	state = RenderState::PreStart;

	Timer t(syncTime);
	std::unique_lock<std::mutex> switch_lock(workerSharedData->mut);
	workerSharedData->cond.wait(switch_lock, [&]() { return workerSharedData->ready == true; });	// wait for worker to finish

	workerSharedData->ready = false; // reset ready flag

	renderingTime = workerSharedData->new_renderTime;	// copy perf data
	drawCallCount = workerSharedData->drawCallCount;	// copy perf data

	state = RenderState::PreStart;
}

void Renderer::flushSubmissions() {
	auto& backBuffer = workerSharedData->renderBuffer;

	backBuffer->camera = frontBuffer->camera;
	backBuffer->resetTextureCache = frontBuffer->resetTextureCache;
	std::swap(backBuffer->textureLoadingQueue, texRefManager.getTextureLoadingQueue());
	if (backBuffer->layers.size() != frontBuffer->layers.size()) {
		backBuffer->layers.resize(frontBuffer->layers.size());
	}
	for (int i = 0; i < frontBuffer->layers.size(); ++i) {
		auto& flayer = frontBuffer->layers[i];	// f(fronbuffer)layer(at i)
		if (flayer.bTemporary) {
			std::swap(backBuffer->layers[i].getDrawables(), flayer.getDrawables());
		}
		else {
			backBuffer->layers[i].copyFrom(flayer);
		}
	}

	/* clear frontbuffer (as it now contains the old backbuffer data):	*/
	texRefManager.clearTextureLoadingQueue();
	frontBuffer->resetTextureCache = false;
	for (auto& l : frontBuffer->layers) {
		if (l.bTemporary) {
			l.clear();
		}
	}
}

void Renderer::startRendering() {
	assertIsState(RenderState::PreStart);
	state = RenderState::PreWait;

	flushSubmissions();
	workerSharedData->cond.notify_one(); // wake up worker
}

void Renderer::reset()
{
	assertIsNotState(RenderState::Uninitialized);
	state = RenderState::PreWait;
	waitTillFinished();
	state = RenderState::Uninitialized;

	workerSharedData->ready = false; // reset ready flag
	workerSharedData->run = false;
	workerSharedData->cond.notify_one(); // wake up worker

	frontBuffer->layers.resize(0);
}
#include "Renderer.hpp"

using namespace std::literals::chrono_literals;

void Renderer::initialize(Window* wndw)
{
	ASSERT_IS_STATE(RenderState::Uninitialized);
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
	ASSERT_IS_STATE(RenderState::PreWait);
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
	frontBuffer->resetTextureCache = false;

	std::swap(backBuffer->textureLoadingQueue, texRefManager.getTextureLoadingQueue());
	texRefManager.clearTextureLoadingQueue();

	// resize backbuffer layer count:
	if (backBuffer->layers.size() != frontBuffer->layers.size()) {
		for (int i = frontBuffer->layers.size(); i < backBuffer->layers.size(); i++) {
			// queue scripts of layers that are destroyed in a resize to a smaller size
			if (backBuffer->layers[i].script) {
				backBuffer->scriptDestructQueue.push_back(std::move(backBuffer->layers[i].script));
			}
		}
		backBuffer->layers.resize(frontBuffer->layers.size());
	}
	// for every layer:
	for (int i = 0; i < frontBuffer->layers.size(); ++i) {
		auto& flayer = frontBuffer->layers[i];	// f(fronbuffer)layer(at i)
		auto& blayer = backBuffer->layers[i];

		//queue scripts for deletion on bScriptDetach flag:
		if (flayer.bScriptDetach && blayer.script) {
			backBuffer->scriptDestructQueue.push_back(std::move(blayer.script));
			blayer.script.release();
			blayer.script = nullptr;
		}
		flayer.bScriptDetach = false;	// reset detach flag
		// attach new scripts from frontbuffer:
		if (flayer.script) {
			// when there is allready a script in place for the layer in backbuffer, we destroy queue it:
			if (blayer.script) {
				backBuffer->scriptDestructQueue.push_back(std::move(blayer.script));
				blayer.script.release();
				blayer.script = nullptr;
			}
			// move script from front to backbuffer:
			blayer.script = std::move(flayer.script);
			flayer.script.release();
			flayer.script = nullptr;
		}


		if (flayer.bTemporary) {
			std::swap(backBuffer->layers[i].getDrawables(), flayer.getDrawables());
			flayer.clear();
		}
		else {
			backBuffer->layers[i].copyFrom(flayer);
		}
	}
}

void Renderer::startRendering() {
	ASSERT_IS_STATE(RenderState::PreStart);
	state = RenderState::PreWait;

	flushSubmissions();
	workerSharedData->cond.notify_one(); // wake up worker
}

void Renderer::reset()
{
	ASSERT_IS_NOT_STATE(RenderState::Uninitialized);

	workerSharedData->ready = false; // reset ready flag
	workerSharedData->run = false;
	workerSharedData->cond.notify_one(); // wake up worker

	state = RenderState::PreWait;
	waitTillFinished();
	state = RenderState::Uninitialized;

	frontBuffer->layers.resize(0);
}
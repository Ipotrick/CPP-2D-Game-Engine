#include "Renderer.hpp"

using namespace std::literals::chrono_literals;

#define ASSERT_IS_STATE(s) \
if (s != this->state) { \
		\
		std::cerr << "in line " << __LINE__ << " ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state SHOULD be " << (int)s << std::endl; \
		exit(-1); \
}

#define ASSERT_IS_NOT_STATE(s)\
	if (s == this->state) {\
	std::cerr << "in line " << __LINE__ <<" ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state SHOULD NOT be " << (int)s << std::endl;\
	exit(-1);\
}

void Renderer::initialize(Window* wndw)
{
	ASSERT_IS_STATE(RenderState::Uninitialized);
	state = RenderState::PreWait;

	window = wndw;
	workerSharedData = std::make_shared<SharedRenderData>();
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
	std::unique_lock switch_lock(workerSharedData->mut);
	workerSharedData->cond.wait(switch_lock, [&]() { return workerSharedData->state == SharedRenderData::State::waitForFrontEnd; });	// wait for worker to finish


	workerSharedData->state = SharedRenderData::State::waitForStartCommand;
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
		auto& flayer = frontBuffer->layers[i];
		auto& blayer = backBuffer->layers[i];

		//queue scripts for deletion on bScriptDetach flag:
		if (flayer.bScriptDetach && blayer.script) {
			backBuffer->scriptDestructQueue.push_back(std::move(blayer.script));
			blayer.script.release();
			blayer.script = nullptr;
		}
		flayer.bScriptDetach = false;	// reset detach flag

		// the onBuffer function is only called for initialized scripts, 
		// the scripts that are added below are initialized only after the next render cycle
		if (blayer.script) {
			blayer.script->onBuffer();
		}

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

		if (flayer.bClearEveryFrame) {
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
	workerSharedData->state = SharedRenderData::State::running;
	workerSharedData->cond.notify_one(); // wake up worker
}

void Renderer::reset()
{
	ASSERT_IS_NOT_STATE(RenderState::Uninitialized);

	workerSharedData->run = false;
	workerSharedData->state = SharedRenderData::State::running;

	workerSharedData->cond.notify_one(); // wake up worker
	state = RenderState::Uninitialized;

	frontBuffer->layers.resize(0);
}
#include "Renderer.hpp"

#include "RenderWorkerThread.hpp"

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

bool Renderer::finished()
{
	return !RenderWorkerThread::exists(jobTag);
}

void Renderer::initialize(Window* window)
{
	ASSERT_IS_STATE(RenderState::Uninitialized);
	state = RenderState::PreWait;

	this->window = window;
	workerSharedData.window = window;
	renderingTime = 0ms;
	syncTime = 0ms;
	frontBuffer = std::make_unique<RenderBuffer>();

	RenderWorkerThread::wait(RenderWorkerThread::submit(&worker, RenderWorkerThread::Action::Init));
	setLayerCount(1);
}

void Renderer::waitTillFinished() {	
	ASSERT_IS_STATE(RenderState::PreWait);
	state = RenderState::PreStart;

	Timer t(syncTime);
	if (RenderWorkerThread::exists(jobTag)) {
		RenderWorkerThread::wait(jobTag);
		jobTag = 0xFFFFFFFFFFFFFFFF;
	}
	renderingTime = workerSharedData.new_renderTime;		// copy perf data
	drawCallCount = workerSharedData.drawCallCount;			// copy perf data
	spriteCountLastFrame = workerSharedData.spriteCount;	// copy perf data

	state = RenderState::PreStart;
}

void Renderer::flushSubmissions() {
	auto& backBuffer = workerSharedData.renderBuffer;

	backBuffer->camera = frontBuffer->camera;

	backBuffer->resetTextureCache = frontBuffer->resetTextureCache;
	frontBuffer->resetTextureCache = false;

	std::swap(backBuffer->textureLoadingQueue, texRefManager.getTextureLoadingQueue());
	texRefManager.clearTextureLoadingQueue();

	// resize backbuffer layer count:
	if (backBuffer->layers.size() != frontBuffer->layers.size()) {
		for (size_t i = frontBuffer->layers.size(); i < backBuffer->layers.size(); i++) {
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

		blayer.copySettings(flayer);

		if (flayer.bClearEveryFrame) {
			std::swap(backBuffer->layers[i].getSprites(), flayer.getSprites());
			flayer.clearSprites();
		}
		else {
			backBuffer->layers[i].setSprites(flayer.getSprites());
		}
	}
}

void Renderer::render() {
	ASSERT_IS_STATE(RenderState::PreStart);
	state = RenderState::PreWait;
	this->window = window;

	flushSubmissions();
	workerSharedData.window = window;
	jobTag = RenderWorkerThread::submit(&worker, RenderWorkerThread::Action::Update);
}

void Renderer::reset()
{
	ASSERT_IS_NOT_STATE(RenderState::Uninitialized);
	state = RenderState::Uninitialized;

	if (RenderWorkerThread::exists(jobTag)) {
		RenderWorkerThread::wait(jobTag);
	}
	RenderWorkerThread::wait(RenderWorkerThread::submit(&worker, RenderWorkerThread::Action::Reset));

	frontBuffer->layers.resize(0);
}

Vec2 Renderer::convertCoordSys(Vec2 coord, RenderSpace from, RenderSpace to) const
{
	switch (from) {
	case RenderSpace::PixelSpace:
		switch (to) {
		case RenderSpace::PixelSpace:
			return coord;
			break;
		case RenderSpace::UniformWindowSpace:
			return convertCoordSys<RenderSpace::PixelSpace, RenderSpace::UniformWindowSpace>(coord);
			break;
		case RenderSpace::WindowSpace:
			return convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(coord);
			break;
		case RenderSpace::WorldSpace:
			return convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WorldSpace>(coord);
			break;
		}
		break;
	case RenderSpace::UniformWindowSpace:
		switch (to) {
		case RenderSpace::PixelSpace:
			return convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::PixelSpace>(coord);
			break;
		case RenderSpace::UniformWindowSpace:
			return coord;
			break;
		case RenderSpace::WindowSpace:
			return convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(coord);
			break;
		case RenderSpace::WorldSpace:
			return convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WorldSpace>(coord);
			break;
		}
		break;
	case RenderSpace::WindowSpace:
		switch (to) {
		case RenderSpace::PixelSpace:
			return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(coord);
			break;
		case RenderSpace::UniformWindowSpace:
			return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(coord);
			break;
		case RenderSpace::WindowSpace:
			return coord;
			break;
		case RenderSpace::WorldSpace:
			return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(coord);
			break;
		}
		break;
	case RenderSpace::WorldSpace:
		switch (to) {
		case RenderSpace::PixelSpace:
			return convertCoordSys<RenderSpace::WorldSpace, RenderSpace::PixelSpace>(coord);
			break;
		case RenderSpace::UniformWindowSpace:
			return convertCoordSys<RenderSpace::WorldSpace, RenderSpace::UniformWindowSpace>(coord);
			break;
		case RenderSpace::WindowSpace:
			return convertCoordSys<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(coord);
			break;
		case RenderSpace::WorldSpace:
			return coord;
			break;
		}
		break;
	}
	return { 0, 0 };	// makes the compiler happy
}


template<> Vec2 Renderer::convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(Vec2 coord) const
{
	return {
		coord.x / window->getWidth() * 2.0f - 1.0f,
		coord.y / window->getHeight() * 2.0f - 1.0f
	};
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(Vec2 coord) const
{
	return (rotate(coord - frontBuffer->camera.position, -frontBuffer->camera.rotation) * frontBuffer->camera.frustumBend * frontBuffer->camera.zoom);
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(Vec2 coord) const
{
	const float xScale = (float)window->getWidth() / (float)window->getHeight();
	coord.x /= xScale;
	return coord;
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(Vec2 coord) const
{
	return {
		(coord.x + 1.0f) / 2.0f * window->getWidth(),
		(coord.y + 1.0f) / 2.0f * window->getHeight()
	};
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(Vec2 coord) const
{
	return rotate(coord / frontBuffer->camera.frustumBend / frontBuffer->camera.zoom, frontBuffer->camera.rotation) + frontBuffer->camera.position;
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(Vec2 coord) const
{
	const float xScale = (float)window->getWidth() / (float)window->getHeight();
	coord.x *= xScale;
	return coord;
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::WorldSpace, RenderSpace::PixelSpace>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(
		convertCoordSys<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(coord));
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WorldSpace>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(
		convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(coord));
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::PixelSpace>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(
		convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(coord));
}
template<> Vec2 Renderer::convertCoordSys<RenderSpace::PixelSpace, RenderSpace::UniformWindowSpace>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(
		convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(coord));
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WorldSpace>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(
		convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(coord));
}

template<> Vec2 Renderer::convertCoordSys<RenderSpace::WorldSpace, RenderSpace::UniformWindowSpace>(Vec2 coord) const
{
	return convertCoordSys<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(
		convertCoordSys<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(coord));
}
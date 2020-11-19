#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Timing.hpp"
#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "RenderBuffer.hpp"
#include "RenderingWorker.hpp"
#include "TextureRefManager.hpp"

#define RENDERER_DEBUG_0

#ifdef RENDERER_DEBUG_0
#define rd_at(x) at(x)
#else 
#define rd_at(x) operator[](x)
#endif

enum class RenderState : int{
	Uninitialized = 0,
	PreWait = 1,
	PreStart = 2,
};

class Renderer {
public:

	void initialize(Window* wndw);
	/*
	* resets all fields to an uninitialised state
	*/
	void reset();

	/*
	* waits for the rendering worker to finish
	*/
	void waitTillFinished();

	/*
	* returns how many layers got added/ deleted
	*/
	int setLayerCount(size_t lc)
	{
		size_t oldSize = frontBuffer->layers.size();
		frontBuffer->layers.resize(lc);
		return (int)frontBuffer->layers.size() - (int)oldSize;
	}
	size_t getLayerCount() const
	{
		return frontBuffer->layers.size();
	}
	/*
	* sets all existing layers to the default RenderLayer
	*/
	void resetLayers()
	{
		for (auto& layer : frontBuffer->layers) {
			layer = RenderLayer();
		}
	}

	RenderLayer& getLayer(int index) { return frontBuffer->layers.rd_at(index); }

	void setLayerBufferTemporary(int index, bool value = true)
	{
		frontBuffer->layers.rd_at(index).bTemporary = value;
	}
	bool isLayerBufferTemporary(int index) const
	{
		return frontBuffer->layers.rd_at(index).bTemporary;
	}

	void submit(Drawable const& d, int layer = 0)
	{
		assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
		assert(frontBuffer->layers.size() > layer);
		frontBuffer->layers.rd_at(layer).push(d);
	}
	void submit(Drawable&& d, int layer = 0)
	{
		assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
		assert(frontBuffer->layers.size() > layer);
		frontBuffer->layers.rd_at(layer).push(d);
	}

	/*
	* writes frontbuffer data to the backbuffer and starts the rendering worker
	*/
	void startRendering();

	void resetTextureCache() { frontBuffer->resetTextureCache = true; }

	// returns the time spend rendering
	std::chrono::microseconds getRenderingTime() { return renderingTime ; }
	// returns the time spend waiting for the worker to finish
	std::chrono::microseconds getWaitedTime() { return syncTime; }
	/*
	* returns the amount of drawcalls the last rendererd frame
	*/
	int getDrawCalls() const { return drawCallCount; }

	TextureRef2 makeTexRef(
		const TextureInfo& texInfo, 
		const Vec2& min = { 0.0f, 0.0f }, 
		const Vec2& max = { 1.0f, 1.0f }) 
	{ 
		return texRefManager.makeRef(texInfo, min, max);
	}

	SmallTextureRef makeSmallTexRef(	// TODO REMOVE
		const TextureInfo& texInfo, 
		const Vec2& min = { 0.0f, 0.0f }, 
		const Vec2& max = { 1.0f, 1.0f }) 
	{ 
		return texRefManager.makeRef(texInfo, min, max).makeSmall();
	}

	/*
	* validaes/ repairs a Texture Ref
	*/
	void validateTextureRef(TextureRef2& ref) { texRefManager.validate(ref); }

	/*
	* converts a position/ vector from one coordinate system (RenderSpace) to an other
	*/
	template<RenderSpace From, RenderSpace To>
	Vec2 convertCoordinate(Vec2 coord) { static_assert(false, "This convertion is not supported");  return coord; }

	Camera& getCamera() { return frontBuffer->camera; }
private:
	void flushSubmissions();

	void assertIsState(RenderState state)
	{
		if (state != this->state) {
			std::cerr << "ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state should be " << (int)state << std::endl;
			exit(-1);
		}
	}

	void assertIsNotState(RenderState state)
	{
		if (state == this->state) {
			std::cerr << "ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state should be " << (int)state << std::endl;
			exit(-1);
		}
	}

	RenderState state{ RenderState::Uninitialized };
	std::chrono::microseconds renderingTime;
	std::chrono::microseconds syncTime;
	int drawCallCount{ 0 };
	TextureRefManager texRefManager;						// TODO refactor, remove loading queue, use frontbuffer loading queue
	// concurrent data:
	std::shared_ptr<RenderBuffer> frontBuffer;				// TODO change to unique_ptr
	std::shared_ptr<RenderingSharedData> workerSharedData;	// TODO change to unique_ptr
	Window* window{ nullptr };
	std::thread workerThread;
};

#define ASSERT_IS_STATE(s)\
	if (s != this->state) {\
	std::cerr << "in line " << __LINE__ <<" ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state SHOULD be " << (int)s << std::endl;\
	exit(-1);\
	}

#define ASSERT_IS_NOT_STATE(s)\
	if (s == this->state) {\
	std::cerr << "in line " << __LINE__ <<" ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state SHOULD NOT be " << (int)s << std::endl;\
	exit(-1);\
	}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::PixelSpace,			RenderSpace::WindowSpace>(Vec2 coord)
{
	return { 
		coord.x / window->width * 2.0f - 1.0f, 
		coord.y / window->height * 2.0f - 1.0f
	};
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WorldSpace,			RenderSpace::WindowSpace>(Vec2 coord)
{
	return (rotate(coord - frontBuffer->camera.position, -frontBuffer->camera.rotation) * frontBuffer->camera.frustumBend * frontBuffer->camera.zoom);
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(Vec2 coord)
{
	const float xScale = (float)window->width / (float)window->height;
	coord.x /= xScale;
	return coord;
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WindowSpace,		RenderSpace::PixelSpace>(Vec2 coord)
{
	return {
		(coord.x + 1.0f) / 2.0f * window->width,
		(coord.y + 1.0f) /2.0f * window->height
	};
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WindowSpace,		RenderSpace::WorldSpace>(Vec2 coord)
{
	return rotate(coord / frontBuffer->camera.frustumBend / frontBuffer->camera.zoom, frontBuffer->camera.rotation) + frontBuffer->camera.position;
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WindowSpace,		RenderSpace::UniformWindowSpace>(Vec2 coord)
{
	const float xScale = (float)window->width / (float)window->height;
	coord.x *= xScale;
	return coord;
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WorldSpace,			RenderSpace::PixelSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(
		convertCoordinate<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(coord));
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::PixelSpace,			RenderSpace::WorldSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(
		convertCoordinate<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(coord));
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::PixelSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(
		convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(coord));
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::PixelSpace,			RenderSpace::UniformWindowSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(
		convertCoordinate<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(coord));
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WorldSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(
		convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(coord));
}
template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WorldSpace,			RenderSpace::UniformWindowSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(
		convertCoordinate<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(coord));
}
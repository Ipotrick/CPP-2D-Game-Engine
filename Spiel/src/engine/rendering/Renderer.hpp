#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../types/Timing.hpp"
#include "../types/BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "RenderBuffer.hpp"
#include "RenderingWorker.hpp"
#include "TextureRefManager.hpp"
#include "../JobSystem.hpp"

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

	bool finished()
	{
		std::unique_lock lock(workerSharedData.mut);
		return workerSharedData.state == SharedRenderData::State::waitForFrontEnd ||
			workerSharedData.state == SharedRenderData::State::reset;
	}

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
		frontBuffer->layers.rd_at(index).bClearEveryFrame = value;
	}
	bool isLayerBufferTemporary(int index) const
	{
		return frontBuffer->layers.rd_at(index).bClearEveryFrame;
	}

	void submit(Sprite const& d, int layer = 0)
	{
		assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
		assert(frontBuffer->layers.size() > layer);
		frontBuffer->layers.rd_at(layer).push(d);
	}
	void submit(Sprite&& d, int layer = 0)
	{
		assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
		assert(frontBuffer->layers.size() > layer);
		frontBuffer->layers.rd_at(layer).push(d);
	}
	void submit(std::vector<Sprite> const& in, int layer = 0)
	{
#ifdef _DEBUG
		for (auto const& d : in) {
			assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
			assert(frontBuffer->layers.size() > layer);
			frontBuffer->layers.rd_at(layer).push(d);
		}
#else
		frontBuffer->layers.rd_at(layer).push(in);
#endif
	}

	/*
	* writes frontbuffer data to the backbuffer and starts the rendering worker
	*/
	void render();

	void resetTextureCache() { frontBuffer->resetTextureCache = true; }

	// returns the time spend rendering
	std::chrono::microseconds getRenderingTime() { return renderingTime ; }
	// returns the time spend waiting for the worker to finish
	std::chrono::microseconds getWaitedTime() { return syncTime; }
	/*
	* returns the amount of drawcalls the last rendererd frame
	*/
	size_t getDrawCallsLastFrame() const { return drawCallCount; }

	size_t getSpriteCountLastFrame() const { return spriteCountLastFrame; }

	BigTextureRef makeTexRef(
		const TextureDiscriptor& texInfo, 
		const Vec2& min = { 0.0f, 0.0f }, 
		const Vec2& max = { 1.0f, 1.0f }) 
	{ 
		return texRefManager.makeRef(texInfo, min, max);
	}

	TextureRef makeSmallTexRef(	// TODO REMOVE
		const TextureDiscriptor& texInfo, 
		const Vec2& min = { 0.0f, 0.0f }, 
		const Vec2& max = { 1.0f, 1.0f }) 
	{ 
		return texRefManager.makeRef(texInfo, min, max).makeSmall();
	}

	/*
	* validaes/ repairs a Texture Ref
	*/
	void validateTextureRef(BigTextureRef& ref) { texRefManager.validate(ref); }

	/**
	 * compile time convertion of coordinate system of vector.
	 * 
	 * \param From RenderSpace the coord is in
	 * \param To RenderSpace the corrd should be converted to
	 * \param vec vector to convert
	 * \return converted vector
	 */
	template<RenderSpace From, RenderSpace To>
	Vec2 convertCoordSys(Vec2 vec) const;

	/**
	 * run time convertion of coordinate system of vector.
	 * 
	 * \param vec vector to convert
	 * \param from RenderSpace the coord is in
	 * \param to RenderSpace the corrd should be converted to
	 * \return converted vector
	 */
	Vec2 convertCoordSys(Vec2 vec, RenderSpace from, RenderSpace to) const;

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
	std::chrono::microseconds renderingTime{ 0 };
	std::chrono::microseconds syncTime{ 0 };
	size_t drawCallCount{ 0 };
	size_t spriteCountLastFrame{ 0 };
	TextureRefManager texRefManager;						// TODO refactor, remove loading queue, use frontbuffer loading queue
	// concurrent data:
	std::shared_ptr<RenderBuffer> frontBuffer;				// TODO change to unique_ptr
	SharedRenderData workerSharedData;	
	Window* window{ nullptr };
	std::thread workerThread;
};

template<> Vec2 Renderer::convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::WorldSpace, RenderSpace::PixelSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::PixelSpace, RenderSpace::WorldSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::PixelSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::PixelSpace, RenderSpace::UniformWindowSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindowSpace, RenderSpace::WorldSpace>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::WorldSpace, RenderSpace::UniformWindowSpace>(Vec2 coord) const;
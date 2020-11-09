#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Timing.hpp"
#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "RenderingWorker.hpp"
#include "TextureRefManager.hpp"

class Renderer {
public:

	void initialize(Window* wndw);

	// waits till the worker thread is finished
	void waitTillFinished();
	// swaps front and backbuffer
	// can only be called when renderer is ready, so allways call waitTillFinished before
	void flushSubmissions();
	// allways call waitTillFinished once after rendering before calling this function
	// submit drawables for the next frame
	void submit(Drawable const& d);
	void submit(Drawable && d);
	// allways call waitTillFinished once after rendering before calling this function
	// wakes up worker to render the scene
	// after calliung this function one MUST call waitTillFinmished before calling any submission function
	void startRendering();

	// ends worker thread
	void end();
	void resetTextureCache() { frontBuffer->resetTextureCache = true; }

	// returns the time spend rendering
	std::chrono::microseconds getRenderingTime() { return renderingTime ; }
	// returns the time spend waiting for the worker to finish
	std::chrono::microseconds getWaitedTime() { return syncTime; }
	/*
	* returns the amount of drawcalls the last rendererd frame
	*/
	int getDrawCalls() const { return drawCallCount; }

	inline TextureRef2 makeTexRef(
		const TextureInfo& texInfo, 
		const Vec2& min = { 0.0f, 0.0f }, 
		const Vec2& max = { 1.0f, 1.0f }) 
	{ 
		return texRefManager.makeRef(texInfo, min, max);
	}

	inline SmallTextureRef makeSmallTexRef(
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
	// for debuging:
	bool wasWaitCalled{ false };	
	bool wasFushCalled{ false };
	bool wasEndCalled{ false };	

	// concurrent data:
	std::shared_ptr<RenderBuffer> frontBuffer;
	std::shared_ptr<RenderingSharedData> workerSharedData;
	Window* window;
	std::thread workerThread;

	// perf:
	std::chrono::microseconds renderingTime;
	std::chrono::microseconds syncTime;
	int drawCallCount{ 0 };

	// texture management:
	TextureRefManager texRefManager;
};

inline void Renderer::submit(Drawable const& d) {
	assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
	frontBuffer->drawables.push_back(d);
}

inline void Renderer::submit(Drawable && d) {
	assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
	frontBuffer->drawables.push_back(d);
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(Vec2 coord)
{
	return { 
		coord.x / window->width * 2.0f - 1.0f, 
		coord.y / window->height * 2.0f - 1.0f
	};
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(Vec2 coord)
{
	Mat3 viewProjectionMatrix = Mat3::scale(frontBuffer->camera.zoom) 
		* Mat3::scale(frontBuffer->camera.frustumBend) 
		* Mat3::rotate(-frontBuffer->camera.rotation) 
		* Mat3::translate(-frontBuffer->camera.position);
	return viewProjectionMatrix * coord;
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(Vec2 coord)
{
	const float xScale = (float)window->width / (float)window->height;
	coord.x /= xScale;
	return coord;
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(Vec2 coord)
{
	return {
		(coord.x + 1.0f) / 2.0f * window->width,
		(coord.y + 1.0f) /2.0f * window->height
	};
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(Vec2 coord)
{
	auto reverseMatrix = Mat3::translate(frontBuffer->camera.position) 
		* Mat3::rotate(frontBuffer->camera.rotation) 
		* Mat3::scale(Vec2(1 / frontBuffer->camera.frustumBend.x, 1 / frontBuffer->camera.frustumBend.y)) 
		* Mat3::scale(1 / frontBuffer->camera.zoom);
	return reverseMatrix * coord;
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(Vec2 coord)
{
	const float xScale = (float)window->width / (float)window->height;
	coord.x *= xScale;
	return coord;
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WorldSpace, RenderSpace::PixelSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(
		convertCoordinate<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(coord));
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::PixelSpace, RenderSpace::WorldSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(
		convertCoordinate<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(coord));
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::PixelSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::PixelSpace>(
		convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(coord));
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::PixelSpace, RenderSpace::UniformWindowSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(
		convertCoordinate<RenderSpace::PixelSpace, RenderSpace::WindowSpace>(coord));
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WorldSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::WorldSpace>(
		convertCoordinate<RenderSpace::UniformWindowSpace, RenderSpace::WindowSpace>(coord));
}

template<> inline Vec2 Renderer::convertCoordinate<RenderSpace::WorldSpace, RenderSpace::UniformWindowSpace>(Vec2 coord)
{
	return convertCoordinate<RenderSpace::WindowSpace, RenderSpace::UniformWindowSpace>(
		convertCoordinate<RenderSpace::WorldSpace, RenderSpace::WindowSpace>(coord));
}
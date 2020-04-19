#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Timing.h"
#include "BaseTypes.h"
#include "RenderTypes.h"
#include "PhysicsTypes.h"
#include "RenderingWorker.h"

class Renderer
{
public:
	Renderer(std::shared_ptr<Window> wndw);

	// waits till the worker thread is finished
	void waitTillFinished();
	// swaps front and backbuffer
	// can only be called when renderer is ready, so allways call waitTillFinished before
	void flushSubmissions();
	// allways call waitTillFinished once after rendering before calling this function
	// submit drawables for the next frame
	void submit(Drawable const& d);
	void submit(Drawable && d);
	// attaches Texture to a drawable
	void attachTex(uint32_t drawableID, std::string_view texName, vec2 min = { 0,0 }, vec2 max = { 1,1 });
	void attachTex(uint32_t drawableID, TextureRef const& texRef);
	// allways call waitTillFinished once after rendering before calling this function
	void setCamera(Camera const& cam);
	// allways call waitTillFinished once after rendering before calling this function
	// wakes up worker to render the scene
	// after calliung this function one MUST call waitTillFinmished before calling any submission function
	void startRendering();

	// ends worker thread
	void end();

	// returns the time spend rendering
	inline std::chrono::microseconds getRenderingTime() { return renderingTime ; }
	// returns the time spend waiting for the worker to finish
	inline std::chrono::microseconds getWaitedTime() { return syncTime; }
private:
	bool wasWaitCalled{ false };
	bool wasFushCalled{ false };
	bool wasEndCalled{ false };
	std::shared_ptr<RenderBuffer> frontBuffer;

	std::shared_ptr<Window> window;
	std::shared_ptr<RenderingSharedData> workerSharedData;
	std::thread workerThread;
	std::chrono::microseconds renderingTime;
	std::chrono::microseconds syncTime;
};

inline void Renderer::submit(Drawable const& d) {
	frontBuffer->drawables.push_back(d);
}

inline void Renderer::submit(Drawable && d) {
#ifdef _DEBUG
	auto d_ = d;
	frontBuffer->drawables.push_back(d);
#else
	frontBuffer->drawables.push_back(d);
#endif
}

inline void Renderer::attachTex(uint32_t drawableID, std::string_view texName, vec2 min, vec2 max)
{
	frontBuffer->newTextureRefs.push_back({ drawableID, TextureRef(texName, min, max) });
}

inline void Renderer::attachTex(uint32_t drawableID, TextureRef const& texRef)
{
	frontBuffer->newTextureRefs.push_back({ drawableID, texRef });
}

inline void Renderer::setCamera(Camera const& cam) {
	frontBuffer->camera = cam;
}
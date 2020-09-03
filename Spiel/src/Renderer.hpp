#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Timing.hpp"
#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "PhysicsTypes.hpp"
#include "RenderingWorker.hpp"
#include "TextureUniforms.hpp"

class Renderer
{
public:
	Renderer(std::shared_ptr<Window> wndw, TextureUniforms& tex);

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

	// Texture utility:
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

	TextureUniforms& tex;
};

inline void Renderer::submit(Drawable const& d) {
	frontBuffer->drawables.push_back(d);
}

inline void Renderer::submit(Drawable && d) {
	frontBuffer->drawables.push_back(d);
}

inline void Renderer::setCamera(Camera const& cam) {
	frontBuffer->camera = cam;
}
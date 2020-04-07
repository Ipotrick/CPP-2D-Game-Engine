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
	// allways call waitTillFinished once after rendering before calling this function
	void submit(Drawable const& d);
	void submit(Drawable && d);
	// allways call waitTillFinished once after rendering before calling this function
	void submitWindowSpace(Drawable const& d);
	void submitWindowSpace(Drawable && d);
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
	inline std::chrono::microseconds getwaitedTime() { return syncTime; }
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<RenderingSharedData> workerSharedData;
	std::thread workerThread;
	std::chrono::microseconds renderingTime;
	std::chrono::microseconds syncTime;
};

inline void Renderer::submit(Drawable const& d) {
	assert(workerSharedData->ready);	// ONLY SUBMIT WHEN WORKER IS WAITING
	workerSharedData->renderBuffer.worldSpaceDrawables.push_back(d);
}

inline void Renderer::submit(Drawable && d) {
	assert(workerSharedData->ready);	// ONLY SUBMIT WHEN WORKER IS WAITING
	workerSharedData->renderBuffer.worldSpaceDrawables.emplace_back(d);
}

inline void Renderer::submitWindowSpace(Drawable const& d) {
	assert(workerSharedData->ready);	// ONLY SUBMIT WHEN WORKER IS WAITING
	workerSharedData->renderBuffer.windowSpaceDrawables.push_back(d);
}

inline void Renderer::submitWindowSpace(Drawable && d) {
	assert(workerSharedData->ready);	// ONLY SUBMIT WHEN WORKER IS WAITING
	workerSharedData->renderBuffer.windowSpaceDrawables.emplace_back(d);
}

inline void Renderer::setCamera(Camera const& cam) {
	assert(workerSharedData->ready);	// ONLY SUBMIT WHEN WORKER IS WAITING
	workerSharedData->renderBuffer.camera = cam;
}
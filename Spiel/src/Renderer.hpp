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
#include "TextureRefManager.hpp"

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
	// allways call waitTillFinished once after rendering before calling this function
	void setCamera(Camera const& cam);
	// allways call waitTillFinished once after rendering before calling this function
	// wakes up worker to render the scene
	// after calliung this function one MUST call waitTillFinmished before calling any submission function
	void startRendering();

	// ends worker thread
	void end();
	void resetTextureCache()
	{
		frontBuffer->resetTextureCache = true;
	}

	// returns the time spend rendering
	std::chrono::microseconds getRenderingTime() { return renderingTime ; }
	// returns the time spend waiting for the worker to finish
	std::chrono::microseconds getWaitedTime() { return syncTime; }
	/*
	* returns the amount of drawcalls the last rendererd frame
	*/
	int getDrawCalls() const { return drawCallCount; }

	inline TextureRef2 makeTexRef(const TextureInfo& texInfo) { return texRefManager.makeRef(texInfo); }

	inline SmallTextureRef makeSmallTexRef(const TextureInfo& texInfo) { return texRefManager.makeRef(texInfo).makeSmall(); }

	// Texture utility:
private:
	// for debuging:
	bool wasWaitCalled{ false };	
	bool wasFushCalled{ false };
	bool wasEndCalled{ false };	

	// concurrent data:
	std::shared_ptr<RenderBuffer> frontBuffer;
	std::shared_ptr<RenderingSharedData> workerSharedData;
	std::shared_ptr<Window> window;
	std::thread workerThread;

	// perf:
	std::chrono::microseconds renderingTime;
	std::chrono::microseconds syncTime;
	int drawCallCount{ 0 };

	// texture management:
	TextureRefManager texRefManager;
};

inline void Renderer::submit(Drawable const& d) {
	/*
	* if yit crashes here, it's most likely, that an orphan SmallTextureRef was created without calling
	* renderer.makeTexRef, and submitted with a drawable. All Texrefs (except for components of entities)
	* must be created my the renderer!.
	*/
	assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
	frontBuffer->drawables.push_back(d);
}

inline void Renderer::submit(Drawable && d) {
	/*
	* if yit crashes here, it's most likely, that an orphan SmallTextureRef was created without calling
	* renderer.makeTexRef, and submitted with a drawable. All Texrefs (except for components of entities)
	* must be created my the renderer!.
	*/
	assert(d.texRef.has_value() ? d.texRef.value().id != -1 : true);
	frontBuffer->drawables.push_back(d);
}

inline void Renderer::setCamera(Camera const& cam) {
	frontBuffer->camera = cam;
}
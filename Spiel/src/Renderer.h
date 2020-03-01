#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>

#include "Camera.h"
#include "Drawable.h"
#include "Window.h"
#include "Entity.h"
#include "World.h"
#include "Timing.h"

class RenderBuffer {
public:
	RenderBuffer() {

	}

	void writeBuffer(World world_, Camera camera_) {
		drawables.clear();
		drawables.reserve(world_.entities.size());
		int i = 0;
		for (Entity el : world_.entities) {
			drawables.emplace_back(world_.entities.at(i++).getDrawable());
		}
		camera = camera_;
	}
public:
	std::vector<Drawable> drawables;
	Camera camera;
};

struct RendererSharedData {
	RendererSharedData(): 
		run{ true },
		ready{ false },
		new_renderTime{ 0 },
		renderBufferB{  },
		mut{  },
		cond{  }
	{}

	bool run;
	bool ready;
	std::chrono::microseconds new_renderTime;
	RenderBuffer renderBufferB;
	std::mutex mut;
	std::condition_variable cond;
};

class Renderer
{
public:
	Renderer(std::shared_ptr<Window> window_, std::shared_ptr<RendererSharedData> sharedData_) :
		window{window_},
		sharedData{ sharedData_ }
	{ }

	void operator()();
	
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<RendererSharedData> sharedData;
};


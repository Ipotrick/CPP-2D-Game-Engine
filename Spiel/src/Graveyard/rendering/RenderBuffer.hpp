#pragma once

#include "Layer.hpp"
#include "Camera.hpp"

struct RenderBuffer {
	std::vector<std::unique_ptr<RenderScript>> scriptDestructQueue;
	bool resetTextureCache{ false };
	Camera camera;
	std::vector<RenderLayer> layers;
};
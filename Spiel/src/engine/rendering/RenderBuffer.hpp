#pragma once

#include "RenderTypes.hpp"
#include "Layer.hpp"

struct RenderBuffer {
	std::vector<BigTextureRef> textureLoadingQueue;
	std::vector<std::unique_ptr<RenderScript>> scriptDestructQueue;
	bool resetTextureCache{ false };
	Camera camera;
	std::vector<RenderLayer> layers;
};
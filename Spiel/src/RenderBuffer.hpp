#pragma once

#include "RenderTypes.hpp"
#include "Layer.hpp"

struct RenderBuffer {

	std::vector<TextureRef2> textureLoadingQueue;
	bool resetTextureCache{ false };
	Camera camera;
	std::vector<RenderLayer> layers;
};
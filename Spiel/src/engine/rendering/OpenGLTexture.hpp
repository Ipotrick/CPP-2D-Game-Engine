#pragma once

#include "RenderTypes.hpp"

struct OpenGLTexture {
	GLuint openGLId{ 0 };
	int width{ -1 };
	int height{ -1 };
	int channelPerPixel{ -1 };
	bool good{ true };
	TextureDiscriptor info;

	void bindToSampler(int samplerSlot);
};
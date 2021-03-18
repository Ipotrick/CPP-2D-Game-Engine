#pragma once

#include <vector>
#include <functional>
#include <mutex>

#include "../OpenGLAbstraction/OpenGLFramebuffer.hpp"
#include "../OpenGLAbstraction/OpenGLPassShader.hpp"

#include "../Camera.hpp"
#include "../Window.hpp"

#include "RenderPipe.hpp"

class RenderPipeline {
public:
	Window* window{ nullptr };
	RenderPipeContext* context{ nullptr };

	virtual void init();
	virtual void exec();
	virtual void reset();
};
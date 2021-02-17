#pragma once

#include <vector>
#include <functional>
#include <mutex>

#include "../OpenGLAbstraction/OpenGLTextureRenderBuffer.hpp"
#include "../OpenGLAbstraction/OpenGLPassShader.hpp"

#include "../Camera.hpp"
#include "../Window.hpp"

#include "RenderPipe.hpp"

struct RenderPipeline {
	Window* window{ nullptr };
	RenderPipeContext* context{ nullptr };
	std::vector<IRenderPipeBackend*> pipes;
};

void exectuePipeline(RenderPipeline& pipeline);

void initPipeline(RenderPipeline& pipeline);

void resetPipeline(RenderPipeline& pipeline);
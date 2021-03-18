#pragma once

#include <mutex>

#include "../OpenGLAbstraction/OpenGLPassShader.hpp"
#include "../Camera.hpp"

struct RenderPipeContext {
	virtual void init()
	{
		mainFrameBuffer.initialize(mainFrameBuffer.getSize().first, mainFrameBuffer.getSize().second);
		passShader.init();
	}
	virtual void reset()
	{
		mainFrameBuffer.reset();
		passShader.reset();
	}
	virtual void update()
	{

	}

	gl::Framebuffer mainFrameBuffer;
	gl::PassShader passShader;

	u32 windowWidth{ 1 };
	u32 windowHeight{ 1 };

	Camera camera;
	float superSampling{ 1.0f };			// move to window class
	bool didFrameSizeChange{ true };
};


class IRenderPipe {
public:
	virtual void init(RenderPipeContext& context) = 0;
	virtual void reset(RenderPipeContext& context) = 0;
	virtual void render(RenderPipeContext& context) = 0;
};
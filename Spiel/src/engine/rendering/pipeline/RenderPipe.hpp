#pragma once

struct RenderPipeContext {
	virtual void init()
	{
		mainFrameBuffer.initialize(mainFrameBuffer.getSize().first, mainFrameBuffer.getSize().second);
		passShader.initialize("shader/PassShader.frag");
	}
	virtual void reset()
	{
		mainFrameBuffer.reset();
		passShader.reset();
	}
	virtual void update()
	{

	}

	OpenGLFrameBuffer mainFrameBuffer;
	OpenGLPassShader passShader;

	u32 windowWidth{ 1 };
	u32 windowHeight{ 1 };

	Camera camera;
	float superSampling{ 1.0f };			// move to window class
	bool didFrameSizeChange{ true };
};


class IRenderPipeBackend {
public:
	virtual void init(RenderPipeContext& context) = 0;
	virtual void reset(RenderPipeContext& context) = 0;
	virtual void render(RenderPipeContext& context) = 0;
};

class IRenderPipe {
public:
	virtual void flush() = 0;
	virtual IRenderPipeBackend* getBackend() = 0;
};
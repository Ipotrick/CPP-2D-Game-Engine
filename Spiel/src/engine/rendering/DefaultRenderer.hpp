#pragma once

#include "pipeline/RenderPipeline.hpp"
#include "pipeline/RenderPipelineThread.hpp"

#include "DefaultPipes/SpriteRenderPipe.hpp"
#include "Font.hpp"

#include "RenderCoordinateSystem.hpp"

class DefaultRenderPipeContext : public RenderPipeContext {
public:
	virtual void init()
	{
		RenderPipeContext::init();
		spriteShaderModelSSBO.initialize(SpriteRenderPipe::Backend::MAX_RECT_COUNT, SpriteRenderPipe::Backend::SPRITE_SHADER_MODEL_SSBO_BINDING);
	}
	virtual void reset()
	{
		RenderPipeContext::reset();
		spriteShaderModelSSBO.reset();
	}

	OpenGLShaderStorage<SpriteShaderModel> spriteShaderModelSSBO;
};

class DefaultRenderer {
public:
	~DefaultRenderer()
	{
		if (bInitialized) {
			reset();
		}
	}
	void init(Window* window) 
	{
		assert(!bInitialized);
		pipeline.window = window;
		pipeline.context = &pipeContext;
		pipeline.pipes.push_back(&spritePipe.backend);
		pipeline.pipes.push_back(&uiSpritePipe.backend);
		bInitialized = true;
		uiSpritePipe.bClearDepthBeforeDraw = true;

		worker.execute(&pipeline, RenderPipelineThread::Action::Init);
	}
	void reset()
	{ 
		assert(bInitialized);

		worker.execute(&pipeline, RenderPipelineThread::Action::Reset);
		worker.wait();
		bInitialized = false;
	}
	void start()
	{
		assert(bInitialized);

		worker.wait();

		flush();

		worker.execute(&pipeline, RenderPipelineThread::Action::Exec);
	}

	void drawSprite(const Sprite& s) { spritePipe.spritesFront.push_back(s); }
	void drawSprite(Sprite&& s) { spritePipe.spritesFront.push_back(std::move(s)); }
	void drawSprites(const std::vector<Sprite>& s) { spritePipe.spritesFront.insert(spritePipe.spritesFront.end(), s.begin(), s.end()); }

	void drawUISprite(const Sprite& s) { uiSpritePipe.spritesFront.push_back(s); }
	void drawUISprite(Sprite&& s) { uiSpritePipe.spritesFront.push_back(std::move(s)); }
	void drawUISprites(const std::vector<Sprite>& s) { uiSpritePipe.spritesFront.insert(uiSpritePipe.spritesFront.end(), s.begin(), s.end()); }

	const RenderCoordSys& getCoordSys() const
	{
		return coordSys;
	}

	f32 supersamplingFactor{ 1.0f };
	Camera camera;
	TextureManager tex;
	FontManager fonts;

private:
	void flush()
	{
		pipeContext.superSampling = supersamplingFactor;
		coordSys = RenderCoordSys{ *pipeline.window, camera };
		tex.getBackend()->flush();
		pipeline.context->camera = camera;
		spritePipe.flush();
		uiSpritePipe.flush();
	}

	RenderCoordSys coordSys;
	bool bInitialized{ false }; 
	RenderPipelineThread worker;
	DefaultRenderPipeContext pipeContext;

	SpriteRenderPipe spritePipe{ tex, pipeContext.spriteShaderModelSSBO };
	SpriteRenderPipe uiSpritePipe{ tex, pipeContext.spriteShaderModelSSBO };

	RenderPipeline pipeline;
};

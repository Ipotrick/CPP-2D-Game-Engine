#pragma once

#include "pipeline/RenderPipeline.hpp"
#include "pipeline/RenderPipelineThread.hpp"

#include "DefaultPipes/SpritePipe.hpp"
#include "Font.hpp"

#include "RenderCoordinateSystem.hpp"

class DefaultRenderPipeContext : public RenderPipeContext {
public:
	virtual void init() override final;
	virtual void reset() override final;
};

struct DefaultRenderPipeline : public RenderPipeline {
	virtual void exec() override final;
	virtual void init() override final;
	virtual void reset() override final;

	void drawBlurTexture();

	std::vector<SpritePipe::Command> sprites;
	std::vector<SpritePipe::Command> uiSprites;

	gl::PassShader blurShader;
	gl::Framebuffer blurTexturePreFramebuffer;
	gl::Framebuffer blurTextureFramebuffer;
	TextureHandle blurTexture;
	SpritePipe* spritePipe{ nullptr };
};

class DefaultRenderer {
public:
	~DefaultRenderer();
	void init(Window* window);
	void reset();
	void start();

	void drawSprite(const Sprite& s) { spriteCommands.push_back(s); }
	void drawSprite(Sprite&& s) { spriteCommands.push_back(std::move(s)); }
	void drawSprites(const std::vector<Sprite>& s) { spriteCommands.insert(spriteCommands.end(), s.begin(), s.end()); }

	void pushCommand(const SpritePipe::Command& cmd) { spriteCommands.push_back(cmd); }
	void pushCommand(SpritePipe::Command&& cmd) { spriteCommands.push_back(cmd); }
	void pushCommands(const std::vector<SpritePipe::Command>& cmd) { spriteCommands.insert(spriteCommands.end(), cmd.begin(), cmd.end()); }

	void drawUISprite(const Sprite& s) { uiSpriteCommands.push_back(s); }
	void drawUISprite(Sprite&& s) { uiSpriteCommands.push_back(std::move(s)); }
	void drawUISprites(const std::vector<Sprite>& s) { uiSpriteCommands.insert(uiSpriteCommands.end(), s.begin(), s.end()); }

	void pushUICommand(const SpritePipe::Command& cmd) { uiSpriteCommands.push_back(cmd); }
	void pushUICommand(SpritePipe::Command&& cmd) { uiSpriteCommands.push_back(cmd); }
	void pushUICommands(const std::vector<SpritePipe::Command>& cmd) { uiSpriteCommands.insert(uiSpriteCommands.end(), cmd.begin(), cmd.end()); }

	const RenderCoordSys& getCoordSys() const { return coordSys; }

	f32 supersamplingFactor{ 1.0f };
	Camera camera;
	TextureManager tex;
	FontManager fonts;

	TextureHandle getBlurTextureHandle() const
	{
		return this->blurTexture;
	}

	f64 sdfBarrier{ 0.02 };

private:
	void flush();

	TextureHandle blurTexture;

	RenderCoordSys coordSys;	
	bool bInitialized{ false }; 
	RenderPipelineThread worker;
	DefaultRenderPipeContext pipeContext;

	std::vector<SpritePipe::Command> spriteCommands;
	std::vector<SpritePipe::Command> uiSpriteCommands;
	SpritePipe spritePipe{ tex.getBackend() };

	DefaultRenderPipeline pipeline;
};

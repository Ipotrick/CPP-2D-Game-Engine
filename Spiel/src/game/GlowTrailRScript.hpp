#pragma once

#include "../engine/RenderScript.hpp"
#include "../engine/OGLTextureRenderBuffer.hpp"
#include "../engine/OGLPassShader.hpp"
#include "../engine/RenderingWorker.hpp"
#include "Game.hpp"

class GlowTrailRScript : public RenderScript {
public:
	virtual void onInitialize(RenderingWorker& render, RenderLayer& layer) override;
	virtual void onUpdate(RenderingWorker& render, RenderLayer& layer) override;
	virtual void onDestroy(RenderingWorker& render) override;
	virtual void onBuffer();
private:
	// trail:
	bool rainbowMode = false;
	float time = 0.0f;
	OGLPassShader pass;
	OGLPassShader trailShader;
	OGLTexFrameBuffer trailFTBO;
	OGLTexFrameBuffer accTrailFTBO;

	// bloom:
	// higher value causes less brighter colors to not bloom
	float bloomBarrier = 1.0f;
	float bloomBufferSizeFactor = 0.5f;
	float minBrightness = 0.81f;
	// kernelWidth controlles how many texels are used for the blurr in one dimension
	int kernelWidth = 30;

	OGLPassShader bloomFinderShader;
	OGLPassShader bloomShader;
	OGLPassShader passShader;

	OGLTexFrameBuffer bloomFBO1;
	OGLTexFrameBuffer bloomFBO2;
	OGLTexFrameBuffer bloomFBO3;
};
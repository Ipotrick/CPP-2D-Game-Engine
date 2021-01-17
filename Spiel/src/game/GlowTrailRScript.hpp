#pragma once

#include "../engine/rendering/RenderScript.hpp"
#include "../engine/rendering/OpenGLTextureRenderBuffer.hpp"
#include "../engine/rendering/OpenGLPassShader.hpp"
#include "../engine/rendering/RenderingWorker.hpp"
#include "Game.hpp"

//class GlowTrailRScript : public RenderScript {
//public:
//	virtual void onInitialize(RenderingWorker& render, RenderLayer& layer) override;
//	virtual void onUpdate(RenderingWorker& render, RenderLayer& layer) override;
//	virtual void onDestroy(RenderingWorker& render) override;
//	virtual void onBuffer();
//private:
//	// trail:
//	bool rainbowMode = false;
//	float time = 0.0f;
//	OGLPassShader pass;
//	OGLPassShader trailShader;
//	OGLTexFrameBuffer trailFTBO;
//	OGLTexFrameBuffer accTrailFTBO;
//
//	// bloom:
//	float bloomBufferSizeFactor = 0.5f;
//	float minBrightness = 0.81f;
//	// kernelWidth controlles how many texels are used for the blurr in one dimension
//	int kernelWidth = 15;
//
//	OGLPassShader bloomFinderShader;
//	OGLPassShader bloomShader;
//	OGLPassShader passShader;
//
//	OGLTexFrameBuffer bloomFBO1;
//	OGLTexFrameBuffer bloomFBO2;
//};
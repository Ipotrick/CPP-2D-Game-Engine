#pragma once

#include "../engine/rendering/RenderScript.hpp"
#include "../engine/rendering/OGLPassShader.hpp"
#include "../engine/rendering/OGLTextureRenderBuffer.hpp"
#include "../engine/rendering/RenderingWorker.hpp"

class BloomRScript : public RenderScript {
public:
	virtual void onInitialize(RenderingWorker& render, RenderLayer& layer) override
	{
		bloomFinderShader.initialize("shader/BloomFinderShader.frag");
		bloomFinderShader.bind();
		glUniform1f(10, bloomBarrier);
		glUniform1f(11, minBrightness);
		passShader.initialize("shader/PassShader.frag");
		bloomShader.initialize("shader/BloomShader.frag"); 
		bloomShader.bind();
		glUniform1i(12, kernelWidth);
		bloomFBO1.initialize();
		bloomFBO2.initialize();
		bloomFBO1.setMagOp(GL_LINEAR);
		bloomFBO1.setMinOp(GL_LINEAR);
		bloomFBO2.setMagOp(GL_LINEAR);
		bloomFBO2.setMinOp(GL_LINEAR);
	}

	virtual void onUpdate(RenderingWorker& render, RenderLayer& layer) override
	{
		auto& mainTFBO = render.getMainTFBO();
		float ssf = render.getSuperSamplingFactor();

		auto [winWidth, winHeight] = render.window->getSize();

		float wWidth = winWidth * ssf;
		float wHeight = winHeight * ssf;
		float bWidth = winWidth * bloomBufferSizeFactor;
		float bHeight = winHeight * bloomBufferSizeFactor;

		glDepthFunc((GLuint)DepthTest::Ignore);

		if (render.didWindowSizeChange()) {
			bloomFBO1.clear();
			bloomFBO1.resize(bWidth, bHeight);
			bloomFBO2.clear();
			bloomFBO2.resize(bWidth, bHeight);
		}

		bloomFBO1.clear();
		bloomFinderShader.bind();
		bloomFinderShader.renderTexToFBO(mainTFBO.getTex(), bloomFBO1);


		bloomShader.bind();
		glUniform1f(10, strength);
		glUniform1i(11, 0);
		bloomFBO2.clear();
		bloomShader.renderTexToFBO(bloomFBO1.getTex(), bloomFBO2);
		bloomShader.bind();
		glUniform1i(11, 1);
		bloomFBO1.clear();
		bloomShader.renderTexToFBO(bloomFBO2.getTex(), bloomFBO1);
		glUniform1i(11, 0);
		bloomFBO2.clear();
		bloomShader.renderTexToFBO(bloomFBO1.getTex(), bloomFBO2);
		glUniform1i(11, 1);
		bloomFBO1.clear();
		bloomShader.renderTexToFBO(bloomFBO2.getTex(), bloomFBO1);

		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
		passShader.renderTexToFBO(bloomFBO1.getTex(), mainTFBO);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	virtual void onDestroy(RenderingWorker& render) override
	{
		bloomFinderShader.reset();
		bloomShader.reset();
		passShader.reset();
		bloomFBO1.reset();
		bloomFBO2.reset();
	}
private:
	// higher value causes less brighter colors to not bloom
	float bloomBarrier = 10;
	float bloomBufferSizeFactor = 0.66f;
	float minBrightness = 0.0f;
	float strength = 1.0f;
	// kernelWidth controlles how many texels are used for the blurr in one dimension
	int kernelWidth = 30;

	OGLPassShader bloomFinderShader;
	OGLPassShader bloomShader;
	OGLPassShader passShader;

	OGLTexFrameBuffer bloomFBO1;
	OGLTexFrameBuffer bloomFBO2;
};
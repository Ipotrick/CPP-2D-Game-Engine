#include "GlowTrailRScript.hpp"

void GlowTrailRScript::onInitialize(RenderingWorker& render, RenderLayer& layer)
{
	// bloom:
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
	bloomFBO3.initialize();
	bloomFBO1.setMagOp(GL_LINEAR);
	bloomFBO1.setMinOp(GL_LINEAR);
	bloomFBO2.setMagOp(GL_LINEAR);
	bloomFBO2.setMinOp(GL_LINEAR);
	bloomFBO3.setMagOp(GL_LINEAR);
	bloomFBO3.setMinOp(GL_LINEAR);

	// trail:
	pass.initialize("shader/FilterPassShader.frag");
	trailShader.initialize("shader/TrailShader.frag");

	trailFTBO.initialize();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	accTrailFTBO.initialize();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void GlowTrailRScript::onUpdate(RenderingWorker& render, RenderLayer& layer)
{
	auto camera = render.getCamera();

	auto& mainTFBO = render.getMainTFBO();
	auto& layerTFBO = render.getThisLayerTFBO();

	auto bWidth = render.window->width * bloomBufferSizeFactor;
	auto bHeight = render.window->height * bloomBufferSizeFactor;

	if (render.didWindowSizeChange()) {
		auto width = render.window->width * render.getSuperSamplingFactor();
		auto height = render.window->height * render.getSuperSamplingFactor();
		//trail:
		trailFTBO.clear();
		trailFTBO.resize(width, height);
		accTrailFTBO.clear();
		accTrailFTBO.resize(width, height);

		// bloom:
		bloomFBO1.clear();
		bloomFBO1.resize(bWidth, bHeight);
		bloomFBO2.clear();
		bloomFBO2.resize(bWidth, bHeight);
		bloomFBO3.clear();
		bloomFBO3.resize(bWidth, bHeight);
	}

	// trail:
	pass.bind();
	glUniform1i(11, true);	// override all alpha
	glUniform1f(10, 0.85f);	// override alpha value
	trailFTBO.clear();
	pass.renderTexToFBO(accTrailFTBO.getTex(), trailFTBO);

	trailShader.bind();
	glUniform1f(10, time * 20);
	glUniform1i(11, rainbowMode);	// rainbowmode
	trailShader.renderTexToFBO(layerTFBO.getTex(), trailFTBO);

	bloomShader.bind();
	glUniform2f(10, bWidth, bHeight);
	glUniform1i(12, 10);
	glUniform1i(11, 0);
	bloomFBO1.clear();
	bloomShader.renderTexToFBO(trailFTBO.getTex(), bloomFBO1);
	glUniform1i(11, 1);
	bloomFBO2.clear();
	bloomShader.renderTexToFBO(bloomFBO1.getTex(), bloomFBO2);

	pass.bind();
	glUniform1i(11, false);	// override all alpha

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	pass.renderTexToFBO(bloomFBO2.getTex(), mainTFBO);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	pass.renderTexToFBO(trailFTBO.getTex(), accTrailFTBO);

	pass.renderTexToFBO(trailFTBO.getTex(), mainTFBO);
	pass.renderTexToFBO(layerTFBO.getTex(), mainTFBO);
}

void GlowTrailRScript::onDestroy(RenderingWorker& render)
{
	//trail:
	pass.reset();
	trailShader.reset();
	trailFTBO.reset();
	accTrailFTBO.reset();

	// bloom:
	bloomFinderShader.reset();
	bloomShader.reset();
	passShader.reset();
	bloomFBO1.reset();
	bloomFBO2.reset();
	bloomFBO3.reset();
}

void GlowTrailRScript::onBuffer()
{
	time = EngineCore::getTotalDeltaTime();
}

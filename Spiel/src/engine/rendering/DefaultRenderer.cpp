#include "DefaultRenderer.hpp"

DefaultRenderer::~DefaultRenderer()
{
	if (bInitialized) {
		reset();
	}
}

void DefaultRenderer::init(Window* window)
{
	assert(!bInitialized);
	pipeline.window = window;
	pipeline.context = &pipeContext;
	pipeline.spritePipe = &spritePipe;
	bInitialized = true;

	blurTexture = pipeline.blurTexture = tex.create(TextureCreateInfo{ std::make_unique<u8>(0) }, std::string("blur"));
	tex.getBackend()->flush();
	worker.execute(&pipeline, RenderPipelineThread::Action::Init);
}

void DefaultRenderer::reset()
{
	assert(bInitialized);

	worker.execute(&pipeline, RenderPipelineThread::Action::Reset);
	worker.wait();
	bInitialized = false;
}

void DefaultRenderer::start()
{
	assert(bInitialized);

	worker.wait();

	flush();

	worker.execute(&pipeline, RenderPipelineThread::Action::Exec);
}

void DefaultRenderer::flush()
{
	pipeContext.superSampling = supersamplingFactor;
	coordSys = RenderCoordSys{ *pipeline.window, camera };
	tex.getBackend()->flush();
	pipeline.context->camera = camera;
	std::swap(pipeline.sprites, spriteCommands);
	spriteCommands.clear();
	std::swap(pipeline.uiSprites, uiSpriteCommands);
	uiSpriteCommands.clear();
}

void DefaultRenderPipeContext::init()
{
	RenderPipeContext::init();
}

void DefaultRenderPipeContext::reset()
{
	RenderPipeContext::reset();
}

void DefaultRenderPipeline::exec()
{
	RenderPipeline::exec();

	spritePipe->push(sprites);
	spritePipe->render(*context);

	drawBlurTexture();

	context->mainFrameBuffer.clearDepth();

	spritePipe->push(uiSprites);
	spritePipe->render(*context);

	context->passShader.renderTexToFBO(context->mainFrameBuffer.getTex(), 0, 0, 0, window->getWidth(), window->getHeight());

	window->swapBuffers();
}

void DefaultRenderPipeline::init()
{
	RenderPipeline::init();
	spritePipe->init(*context);

	const auto fragBlurr = readShader("shader/BlurShader.frag");
	blurShader.init(fragBlurr.c_str());

	blurTexturePreFramebuffer.initialize(1, 1);
	spritePipe->texLock.lock();
	spritePipe->texBackend->createAndLoadQueueExecute(spritePipe->texLock);
	blurTextureFramebuffer.initialize(spritePipe->texBackend->get(spritePipe->texLock, blurTexture.index), 1, 1);
	spritePipe->texLock.unlock();
}

void DefaultRenderPipeline::reset()
{
	spritePipe->reset(*context);
	RenderPipeline::reset();
}

void DefaultRenderPipeline::drawBlurTexture()
{
	spritePipe->texLock.lock();
	if (context->mainFrameBuffer.getSize() != blurTextureFramebuffer.getSize()) {
		blurTextureFramebuffer.resize(context->mainFrameBuffer.getSize().first, context->mainFrameBuffer.getSize().second);
		blurTexturePreFramebuffer.resize(context->mainFrameBuffer.getSize().first, context->mainFrameBuffer.getSize().second);
	}

	int screenWidth = context->mainFrameBuffer.getSize().first;
	int screenHeight = context->mainFrameBuffer.getSize().second;
	int horizontal{ -1 };
	int blurStepSize{ 10 };
	int blurSteps{ 10 };
	blurShader.setUniform(52, &screenWidth);
	blurShader.setUniform(53, &screenHeight);
	blurShader.setUniform(54, &blurStepSize);
	blurShader.setUniform(55, &blurSteps);

	blurTexturePreFramebuffer.clear();
	horizontal = false;
	blurShader.setUniform(51, &horizontal);
	blurShader.renderTexToFBO(context->mainFrameBuffer.getTex(), blurTexturePreFramebuffer);
	blurTextureFramebuffer.clear();

	horizontal = true;
	blurShader.setUniform(51, &horizontal);
	blurShader.renderTexToFBO(blurTexturePreFramebuffer.getTex(), blurTextureFramebuffer);
	blurTexturePreFramebuffer.clear();

	spritePipe->texLock.unlock();
}

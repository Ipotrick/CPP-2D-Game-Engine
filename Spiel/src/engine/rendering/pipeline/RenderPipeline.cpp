#include "RenderPipeline.hpp"

#include <gl/glew.h>

void openGLDebugMessageCallback3(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* msg, const void* data)
{
	const char* _source;
	const char* _type;
	const char* _severity;

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		_source = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN"; break;
	default:
		_source = "UNKNOWN"; break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR"; break;
	case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER"; break;
	case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER"; break;
	default:
		_type = "UNKNOWN"; break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM"; break;
	case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION"; break;
	default:
		_severity = "UNKNOWN"; break;
	}

	if (std::strcmp(_severity, "NOTIFICATION")) {
		printf("OpenGL error [%d]: %s of %s severity, raised from %s: %s\n",
			id, _type, _severity, _source, msg);
		//__debugbreak();
	}
}

void exectuePipeline(RenderPipeline& pipeline)
{
	cu32 currWindowWidth = pipeline.window->getWidth();
	cu32 currWindowHeight = pipeline.window->getHeight();
	if (pipeline.context->windowWidth != currWindowWidth * pipeline.context->superSampling || 
		pipeline.context->windowHeight != currWindowHeight * pipeline.context->superSampling) 
	{
		pipeline.context->mainFrameBuffer.resize(currWindowWidth * pipeline.context->superSampling, currWindowHeight * pipeline.context->superSampling);
		pipeline.context->didFrameSizeChange = true;
		pipeline.context->windowWidth = currWindowWidth;
		pipeline.context->windowHeight = currWindowHeight;
	}
	else {
		pipeline.context->didFrameSizeChange = false;
	}

	pipeline.context->mainFrameBuffer.clear();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& pipe : pipeline.pipes) {
		pipe->render(*pipeline.context);
	}

	pipeline.context->passShader.renderTexToFBO(pipeline.context->mainFrameBuffer.getTex(), 0, pipeline.window->getWidth(), pipeline.window->getHeight());

	pipeline.window->swapBuffers();
}

void initPipeline(RenderPipeline& pipeline)
{
	pipeline.window->takeRenderingContext();

	// opengl configuration:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(openGLDebugMessageCallback3, 0);
#endif

	pipeline.context->init();
	for (IRenderPipeBackend* backend : pipeline.pipes) {
		backend->init(*pipeline.context);
	}
}

void resetPipeline(RenderPipeline& pipeline)
{
	pipeline.context->reset();
	for (IRenderPipeBackend* backend : pipeline.pipes) {
		backend->reset(*pipeline.context);
	}
	pipeline.window->returnRenderingContext();
}

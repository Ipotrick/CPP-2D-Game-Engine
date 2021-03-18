#include "OpenGLFrameBuffer.hpp"

namespace gl {
	void Framebuffer::initialize(uint32_t width, uint32_t height)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glEnablei(GL_BLEND, fbo);

		bOwningTexture = true;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 1, 1);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void Framebuffer::initialize(OpenGLTexture& tex, uint32_t width, uint32_t height)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glEnablei(GL_BLEND, fbo);

		bOwningTexture = false;
		this->tex = tex.openGLId;
		glBindTexture(GL_TEXTURE_2D, this->tex);

		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 1, 1);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->tex, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void Framebuffer::clear()
	{
		constexpr GLfloat clearColor[4] = { 0, 0, 0, 0 };
		constexpr GLfloat clearDepth[1] = { std::numeric_limits<float>::max() };
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearBufferfv(GL_COLOR, 0, clearColor);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glClearBufferfv(GL_DEPTH, 0, clearDepth);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void Framebuffer::clearDepth()
	{
		constexpr GLfloat clearDepth[1] = { std::numeric_limits<float>::max() };
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glClearBufferfv(GL_DEPTH, 0, clearDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void Framebuffer::reset()
	{
		if (fbo != -1) {
			glDeleteBuffers(1, &depthBuffer);
			depthBuffer = -1;
			if (bOwningTexture) {
				glDeleteTextures(1, &tex);
			}
			tex = -1;
			glDeleteBuffers(1, &fbo);
			fbo = -1;
		}
	}


	void Framebuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, getBuffer());
		GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
	}

	void Framebuffer::resize(uint32_t width, uint32_t height)
	{
		assert(fbo != -1);
		this->width = width;
		this->height = height;
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	}

	void Framebuffer::setMinOp(GLuint op)
	{
		assert(fbo != -1);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, op);
	}

	void Framebuffer::setMagOp(GLuint op)
	{
		assert(fbo != -1);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, op);
	}
}
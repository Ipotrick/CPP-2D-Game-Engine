#include "OpenGLTexture.hpp"

#include "GLFW/glfw3.h"

void OpenGLTexture::bindToSampler(int samplerSlot)
{
	glActiveTexture(GL_TEXTURE0 + samplerSlot);
	glBindTexture(GL_TEXTURE_2D, openGLId);
}
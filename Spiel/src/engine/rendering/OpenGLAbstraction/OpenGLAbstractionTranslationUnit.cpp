#include "DepthTest.hpp"
#include "Blending.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace gl {
	void enableBlending()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// default blend setting
	}

	void setBlendingEquasion(BlendingEquasion eq)
	{
		glBlendEquation(static_cast<u32>(eq));
	}

	void setBlendingFactors(BlendingFactor sourceFactor, BlendingFactor destinationFactor)
	{
		glBlendFunc(static_cast<u32>(sourceFactor), static_cast<u32>(destinationFactor));
	}

	void setBlending(BlendingFunction setting)
	{
		setBlendingFactors(setting.srcFactor, setting.dstFactor);
		setBlendingEquasion(setting.equasion);
	}

	void enableDepthTesting()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	void setDepthTest(DepthTest depthTest)
	{
		glDepthFunc(static_cast<u32>(depthTest));
	}
};

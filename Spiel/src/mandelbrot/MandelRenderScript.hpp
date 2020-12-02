#pragma once

#include "MandelbrotViewer.hpp"
#include "../engine/RenderScript.hpp"
#include "../engine/RenderingWorker.hpp"

class MandelRenderScript : public RenderScript {
public:

	virtual void onInitialize(RenderingWorker& render, RenderLayer& layer)
	{
		mandelShader.initialize("shader/Mandel.frag");
	}
	virtual void onUpdate(RenderingWorker& render, RenderLayer& layer) 
	{
		auto cam = render.getCamera();
		Mat4 vp =
			Mat4::translate({ cam.position.x, cam.position.y, 0.0f }) *
			Mat4::scale({ 1 / cam.frustumBend.x, 1 / cam.frustumBend.y, 1.0f }) *
			Mat4::scale(1 / cam.zoom);

		mandelShader.bind();
		glUniformMatrix4fv(1, 1, GL_FALSE, vp.data());
		mandelShader.renderToFBO(render.getMainTFBO());
	}
	virtual void onDestroy(RenderingWorker& render) 
	{
		mandelShader.reset();
	}
private:
	OGLPassShader mandelShader;
};
#pragma once

#include "../pipeline/RenderPipe.hpp"


class SpriteRenderPipe : public IRenderPipe {
public:

	class Backend : public IRenderPipeBackend {
	public:
		virtual void init(RenderPipeContext& context) override;
		virtual void reset(RenderPipeContext& context) override;
		virtual void render(RenderPipeContext& context) override;
	};

	virtual void flush() override
	{
	}
	virtual IRenderPipeBackend* getBackend()  override
	{
	}
};


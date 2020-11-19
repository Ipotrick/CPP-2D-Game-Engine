#pragma once

#include <optional>

#include "RenderTypes.hpp"
#include "RenderScript.hpp"

class RenderingWorker;

class RenderLayer {
public:
	void push(Drawable&& d) { drawables.push_back(d); }
	void push(Drawable const& d) { drawables.push_back(d); }

	std::vector<Drawable>& getDrawables()
	{
		return drawables;
	}

	void clear() 
	{ 
		drawables.clear();
	}

	void copyFrom(RenderLayer const& rhs)
	{
		drawables = rhs.drawables;
	}

	void attachRenderScript(std::unique_ptr<RenderScript>&& script)
	{
		this->script = std::move(script);	// give over new script
	}
	void detachRenderScript()
	{
		this->bScriptDetach = true;
		if (this->script) {
			this->script = nullptr;
		}
	}

	bool bEnable{ true };
	bool bSortForDepth{ false };
	bool bStableSort{ true };
	bool bTemporary{ true };	// if set to false, the buffer will not be cleared between frames.

	RenderSpace renderMode{ RenderSpace::WindowSpace };

private:
	friend class Renderer;
	friend class RenderingWorker;

	std::vector<Drawable> drawables;

	bool bScriptDetach{ false };	// signal for destroying current Script
	// temporary storage for the frontBuffer
	// but permanent storage for backbuffer
	std::unique_ptr<RenderScript> script{ nullptr };	
};
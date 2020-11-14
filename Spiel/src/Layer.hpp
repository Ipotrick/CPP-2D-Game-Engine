#pragma once

#include "RenderTypes.hpp"

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

	bool bEnable{ true };
	bool bSortForDepth{ false };
	bool bStableSort{ true };
	bool bTemporary{ true };	// if set to false, the buffer will not be cleared between frames.

	RenderSpace renderMode{ RenderSpace::WindowSpace };
private:
	std::vector<Drawable> drawables;
};
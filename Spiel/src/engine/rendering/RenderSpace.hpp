#pragma once

#include <cassert>

#include <iostream>

enum class RenderSpace : char {
	/* world coordinates, (0,0) is world's (0,0) */
	Camera,
	/* window (-1 to 1 in x and y) cooordinates, (0,0) is middle of the window */
	Window,
	/* 
	* window coordinates that ignore aspect ratio,
	* eg y coordinates are the same as in window space,
	* but the x coordinates are scaled so that they stride
	* is the same of the y axis
	*/
	UniformWindow,
	/* coordinates are pixels, (0,0) is lower left corner */
	Pixel
};

inline std::string renderSpaceToStr(RenderSpace rs)
{
	switch (rs) {
	case RenderSpace::Camera:
		return "WorldSpace";
	case RenderSpace::Window:
		return "WindowSpace";
	case RenderSpace::UniformWindow:
		return "UniformWindowSpace";
	case RenderSpace::Pixel:
		return "PixelSpace";
	default:
		assert(false);
		return "";
	}
}

inline std::ostream& operator<<(std::ostream& ostream, RenderSpace rs)
{
	ostream << renderSpaceToStr(rs);
	return ostream;
}
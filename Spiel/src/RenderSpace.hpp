#pragma once

enum class RenderSpace : char {
	/*world coordinates, (0,0) is world's (0,0)*/
	WorldSpace,
	/* window (-1 to 1 in x and y) cooordinates, (0,0) is middle of the window */
	WindowSpace,
	/* window coordinates that ignore aspect ratio, (0,0) is middle of the window*/
	UniformWindowSpace,
	/* coordinates are pixels, (0,0) is lower left corner */
	PixelSpace
};
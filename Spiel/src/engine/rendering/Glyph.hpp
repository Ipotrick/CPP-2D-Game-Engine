#pragma once

#include <cmath>
#include <iostream>

#include "../types/ShortNames.hpp"

struct Glyph {
private:
	struct Bounds {
		f32 left{ NAN };
		f32 bottom{ NAN };
		f32 right{ NAN };
		f32 top{ NAN };
	};
public:
	f32 advance{ NAN };
	Bounds planeBounds;
	Bounds atlasBounds;
};

Glyph scaleGlyph(Glyph g, f32 texWidth, f32 texHeight);

std::ostream& operator<<(std::ostream& os, const Glyph& g);
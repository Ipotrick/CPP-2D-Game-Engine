#pragma once

#include "GUIElement.hpp"

namespace gui {

	struct Style {
		Vec4 fill0;
		Vec4 fill1;
		Vec4 fill2;
		Vec4 accent0;
		Vec4 positive;
		Vec4 negative;
		Padding padding;
		float spacing;
	};

	static inline const Style DEFAULT_STYLE{
		.fill0 = Vec4::From255(0x15, 0x15, 0x16, 0xFF),
		.fill1 = Vec4::From255(0x24, 0x24, 0x25, 0xFF),
		.fill2 = Vec4::From255(0x104, 0x104, 0x110, 0xFF),
		.accent0 = Vec4::From255(0xD9, 0xB0, 0x8C, 0xFF),
		.positive = Vec4::From255(30, 100, 40, 0xFF),
		.negative = Vec4::From255(100, 30, 40, 0xFF),
		.padding = Padding{5,5,5,5},
		.spacing = 5.0f,
	};

	template<CElement T>
	void applyStyle(T& element, const Style& style) {}

}

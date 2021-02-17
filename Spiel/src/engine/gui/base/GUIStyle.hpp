#pragma once

#include "GUIElement.hpp"
#include "../../rendering/Font.hpp"
#include "../../rendering/OpenGLAbstraction/OpenGLTexture.hpp"

namespace gui {

	struct Style {
		Vec4 fill0;
		Vec4 fill1;
		Vec4 fill2;
		Vec4 accent0;
		Vec4 positive;
		Vec4 negative;
		f32 fontSize{ 14 };
		Vec4 fontColor1{ 0,0,0,1 };
		Vec4 fontColor2{ 1,1,1,1 };
		Padding padding;
		f32 spacing;
		FontDescriptor font;
		TextureDescriptor fontTex;
	};

	static inline const Style DEFAULT_STYLE{
		.fill0 = Vec4::From255(0x15, 0x15, 0x16, 0xFF),
		.fill1 = Vec4::From255(0x24, 0x24, 0x25, 0xFF),
		.fill2 = Vec4::From255(0x104, 0x104, 0x110, 0xFF),
		//.accent0 = Vec4::From255(0xD9, 0xB0, 0x8C, 0xFF),
		.accent0 = Vec4{0.37,0.37,0.37, 1},
		.positive = Vec4::From255(30, 100, 40, 0xFF),
		.negative = Vec4::From255(100, 30, 40, 0xFF),
		.fontSize = 12.0f,
		.fontColor1 = Vec4{ 1, 1, 1, 1 },
		.fontColor2 = Vec4{ 0, 0, 0, 1 },
		.padding = Padding{5,5,5,5},
		.spacing = 5.0f,
		.font = FontDescriptor{"ressources/fonts/Consolas_font.csv"},
		.fontTex = TextureDescriptor{"ressources/fonts/Consolas_font.png", TexFilter::Linear, TexFilter::Linear},
	};

	template<CElement T>
	void applyStyle(T& element, const Style& style) {}
}

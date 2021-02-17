#pragma once

#include "GUITextElements.hpp"
#include "../base/GUIStyle.hpp"

namespace gui {

	template<> inline void applyStyle(TextInputF64& self, const Style& style)
	{
		if (hasNANS(self.color)) self.color = Vec4(1,1,1, 1);
		if (hasNANS(self.colorFont)) self.colorFont = style.fontColor2;
		if (hasNANS(self.colorFontError)) self.colorFontError = style.negative;
		if (hasNANS(self.textPadding)) self.textPadding = Padding{ 0.0f, 0.0f, style.padding.left * 0.5f, style.padding.right * 0.5f };
		if (!self.fontPair.first.holdsValue()) self.fontPair.first = style.font;
		if (!self.fontTexPair.first.holdsValue()) self.fontTexPair.first = style.fontTex;
		if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
	}

	template<> inline void applyStyle(StaticText& self, const Style& style) {
		if (hasNANS(self.color)) self.color = style.fontColor1;
		if (!self.fontPair.first.holdsValue()) self.fontPair.first = style.font;
		if (!self.fontTexPair.first.holdsValue()) self.fontTexPair.first = style.fontTex;
		if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
	}

	template<> inline void applyStyle(Text& self, const Style& style) {
		if (hasNANS(self.color)) self.color = style.fontColor1;
		if (!self.fontPair.first.holdsValue()) self.fontPair.first = style.font;
		if (!self.fontTexPair.first.holdsValue()) self.fontTexPair.first = style.fontTex;
		if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
	}

	template<> inline void applyStyle(TextInput& self, const Style& style) {
		if (hasNANS(self.color)) self.color = style.fontColor2;
		if (!self.fontPair.first.holdsValue()) self.fontPair.first = style.font;
		if (!self.fontTexPair.first.holdsValue()) self.fontTexPair.first = style.fontTex;
		if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
	}
}
#pragma once

#include "GUITextElements.hpp"
#include "../base/GUIStyle.hpp"

namespace gui {

	template<> inline void applyStyle(TextInputF64& self, const Style& style)
	{
		if (hasNANS(self.color)) self.color = Vec4(1,1,1, 1);
		if (hasNANS(self.colorFont)) self.colorFont = Vec4(0,0,0, 1);
		if (hasNANS(self.colorFontError)) self.colorFontError = style.negative;
	}
}
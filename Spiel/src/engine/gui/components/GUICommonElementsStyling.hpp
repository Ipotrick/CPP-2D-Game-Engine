#pragma once

#include "GUICommonElements.hpp"
#include "../base/GUIStyle.hpp"

namespace gui {

	template<> inline void applyStyle(Column& self, const Style& style)
	{
		if (hasNANS(self.padding)) self.padding = Padding{0,0,0,0};		// Columns have no padding per default
		if (std::isnan(self.spacing)) self.spacing = style.spacing;
	}

	template<> inline void applyStyle(DropBox& self, const Style& style)
	{
		if (hasNANS(self.color)) self.color = style.fill1;
	}

	//template<> inline void applyStyle(DragDroppable& self, const Style& style)
	//{
	//	if (hasNANS(self.color)) self.color = style.accent0;
	//}
	
	template<> inline void applyStyle(Row& self, const Style& style)
	{
		if (hasNANS(self.padding)) self.padding = Padding{ 0,0,0,0 };		// Columns have no padding per default
		if (std::isnan(self.spacing)) self.spacing = style.spacing;
	}

	template<> inline void applyStyle(Box& self, const Style& style)
	{
		if (hasNANS(self.padding)) self.padding = DEFAULT_STYLE.padding;
		if (hasNANS(self.color)) self.color = (self.bFillSpace ? style.fill0 : style.fill1);
	}

	template<> inline void applyStyle(Button& self, const Style& style)
	{
		if (hasNANS(self.color)) self.color = style.accent0;
			if (hasNANS(self.holdColor)) self.holdColor = style.positive;
	}

	template<> inline void applyStyle(Checkbox& self, const Style& style)
	{
		if (hasNANS(self.color)) self.color = style.fill1;
		if (hasNANS(self.colorEnabled)) self.colorEnabled = style.positive;
		if (hasNANS(self.colorDisabled)) self.colorDisabled = style.negative;
	}

	template<> inline void applyStyle(SliderF64& self, const Style& style)
	{
		if (hasNANS(self.colorBar)) self.colorBar = style.fill1;
		if (hasNANS(self.colorSlider)) self.colorSlider = style.accent0;
		if (hasNANS(self.colorError)) self.colorError = style.negative;
	}

	template<> inline void applyStyle(Footer& self, const Style& style)
	{
		if (std::isnan(self.spacing)) self.spacing = style.spacing;
	}
}

#pragma once

#include <vector>

#include "GUIDrawContext.hpp"
#include "GUIDrawUtil.hpp"
#include "GUIElement.hpp"

namespace gui {

	struct Root {
		Sizeing sizeing;
		Placeing placeing;
		Vec4 color{ 0,0,0,1 };
		u32 child{ INVALID_ELEMENT_ID };
	};
}

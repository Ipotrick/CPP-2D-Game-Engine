#pragma once

#include <vector>

#include "GUIDrawContext.hpp"
#include "GUIDrawUtil.hpp"
#include "GUIElement.hpp"

namespace gui {

	struct Root {
		Sizing sizeing;
		Placing placeing;
		u32 child{ INVALID_ELEMENT_ID };
	};
}

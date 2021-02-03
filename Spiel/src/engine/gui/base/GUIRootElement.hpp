#pragma once

#include <vector>

#include "GUIDrawContext.hpp"
#include "GUIDrawUtil.hpp"
#include "GUIElement.hpp"

namespace gui {

	struct Root {
		std::function<void(Root&)> onUpdate;
		Sizing sizing;
		Placing placing;
		u32 child{ INVALID_ELEMENT_ID };
	};
}

#pragma once

#include <vector>

#include "../base/GUIElement.hpp"

namespace gui {
	struct Column : IElement {
		Vec2 minsize{ 10.0f, 10.0f };
		XAlign xalignment{ XAlign::Left };
		YAlign yalignment{ YAlign::Top };
		Listing listing{ Listing::Packed };
		float spaceing{ 0.0f };
		std::vector<u32> children;
	};
}

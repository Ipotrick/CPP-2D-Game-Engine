#pragma once

#include <vector>

#include "../base/GUIElement.hpp"

namespace gui {

	struct Box : IElement {
		Vec2 minsize{ 10,10 };
		Vec4 color{ 1, 0.33, 0.66, 1 };
	};

	struct FillBox : IElement {
		Vec4 color{ 
			rand() % 1000 / 1000.0f, 
			rand() % 1000 / 1000.0f,
			rand() % 1000 / 1000.0f,
			1.0f 
		};
	};
}

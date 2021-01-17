#pragma once

#include "../base/GUIElement.hpp"

namespace gui {

	struct Container {
		Vec4 color{ 0,0,0,1 };
		uint32_t childId{ 0xFFFFFFFF };
	};

	class ContainerElement : public IElement {
	public:
		ContainerElement(Container const& param)
		{
			self = param;
		}
	private:
		Container self;
	};
}

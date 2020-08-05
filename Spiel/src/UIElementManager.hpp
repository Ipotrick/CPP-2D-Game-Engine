#pragma once

#include "RenderTypes.hpp"
#include <deque>

namespace ui {

	class Element {
	protected:
		Element* child{ nullptr };
		Element* parent{ nullptr };
	public:
	};

	class RootElement : public Element {
	protected:
	public:
	};

	class StyleFrame : public Element {

	};

	class ElementManager {
		std::vector<RootElement> rootElements;
		std::deque<uint32_t> rootElementsFreeQueue;

		std::vector<StyleFrame> styleFrame;
		std::deque<uint32_t> styleFrameFreeQueue;
	public:
		
	};
}
#pragma once

#include "GUIDrawContext.hpp"
#include "GUIDrawUtil.hpp"

namespace gui {

	static inline constexpr u32 INVALID_ELEMENT_ID{ 0xFFFFFFFF };

	class IElement {};

	template<typename T>
	concept CElement = std::is_base_of_v<IElement, T>;
};
#pragma once

#include "../../types/ShortNames.hpp"

namespace gl {

	enum class DepthTest : u32 {
		Never = 0x0200,
		Less = 0x0201,
		Equal = 0x0202,
		LessEqual = 0x0203,
		Greater = 0x0204,
		NotEqual = 0x0205,
		GreaterEqual = 0x0206,
		Always = 0x0207
	};

	void enableDepthTesting();

	void setDepthTest(DepthTest depthTest);
};
#pragma once

#include "../../types/ShortNames.hpp"

struct RessourceHandleBase {
	u32 index{ 0xFFFFFFFF };
	u16 version{ 0xFFFF };
	u16 managerId{ 0xFFFF };

	bool holdsValue() const { return index != 0xFFFFFFFF && version != 0xFFFF && managerId != 0xFFFF; }
};

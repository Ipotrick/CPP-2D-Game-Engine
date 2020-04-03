#pragma once

#include <cstdint>
#include <ostream>

#include "glmath.h"

enum class Form : uint8_t {
	CIRCLE = 0x00,
	RECTANGLE = 0x01
};

inline std::ostream& operator<<(std::ostream& ios, Form form) {
	ios << ((bool)form ? "Rectangle" : "Circle");
	return ios;
}
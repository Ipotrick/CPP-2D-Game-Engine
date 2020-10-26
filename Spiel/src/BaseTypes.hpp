#pragma once

#include <cstdint>
#include <ostream>
#include <string>

enum class Form : uint8_t {
	Circle = 0x00,
	Rectangle = 0x01
};

static inline std::string formToString(Form form)
{
	if (form == Form::Circle) {
		return "Circle";
	}
	else {
		return "Rectangle";
	}
}

static inline Form stringToForm(std::string const& str)
{
	if (str == "Circle") {
		return Form::Circle;
	}
	else {
		return Form::Rectangle;
	}
}

inline std::ostream& operator<<(std::ostream& ios, Form form) {
	ios << ((bool)form ? "Rectangle" : "Circle");
	return ios;
}
 #pragma once

#include <string>

enum class Focus {
	Out,		// Application is not focused
	Global,		// Application is in focus, the specific focis is neglected
	Standard,	// Application is in Standard opertaional Mode
	WriteText,	// Application is in the focus of a textbox or textfield
	UI			// Application is in the focus of a menu and other parts of UI
};

static inline std::string focusToString(Focus focus)
{
	switch (focus) {
	case Focus::Out:
		return "Out";
	case Focus::Global:
		return "Gloabal";
	case Focus::Standard:
		return "Standard";
	case Focus::WriteText:
		return "WriteText";
	case Focus::UI:
		return "Menu";
	default:
		assert(false);
		return "";
	}
}
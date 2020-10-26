#pragma once

enum class Focus {
	Out,		// Application is not focused
	Global,		// Application is in focus, the specific focis is neglected
	Standard,	// Application is in Standard opertaional Mode
	WriteText,	// Application is in the focus of a textbox or textfield
	UI			// Application is in the focus of a menu and other parts of UI
};
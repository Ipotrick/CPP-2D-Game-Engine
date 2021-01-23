#pragma once

#include "../base/GUIElement.hpp"

namespace gui {

	struct StaticText : IElement {
		char const* str{ nullptr };
		Vec2 fontSize{ 10,20 };
		BigTextureRef fonttexture{ "Consolas.png" };
	};

	struct TextInput : IElement {
		Vec2 size{ 100,20 };
		Vec2 fontSize{ 10,20 };
		BigTextureRef fonttexture{ "Consolas.png" };
		std::function<void(std::string)> onStore;
		bool bClearOnEnter{ false };
	};
	namespace {
		struct _TextInput : TextInput {
			_TextInput(TextInput&& e) : TextInput{e} {}
			std::string str;
		};
	}
}

#pragma once

#include "../base/GUIElement.hpp"

namespace gui {

	struct StaticText : IElement {
		std::function<void(StaticText&)> onUpdate;
		std::string_view value;
		Vec2 fontSize{ 10,20 };
		Vec4 color{ 0,0,0,1 };
		BigTextureRef fonttexture{ "ConsolasSoft.png" };
	};

	struct Text : IElement {
		std::function<void(Text&)> onUpdate;
		std::string* value{ nullptr };
		Vec2 fontSize{ 10,20 };
		Vec4 color{ 0,0,0,1 };
		BigTextureRef fonttexture{ "ConsolasSoft.png" };
	};

	struct TextInput : IElement {
		std::function<void(TextInput&)> onUpdate;
		Vec2 size{ 100,20 };
		XAlign xalign{ XAlign::Left };
		YAlign yalign{ YAlign::Top };
		Vec2 fontSize{ 10,20 };
		Vec4 color{ 0,0,0,1 };
		BigTextureRef fonttexture{ "ConsolasSoft.png" };
		std::function<void(std::string)> onStore;
		bool bClearOnEnter{ false };
	};
	namespace {
		struct _TextInput : public TextInput {
			_TextInput(TextInput&& e) : TextInput{e} {}
			std::string str;
		};
	}

	struct TextInputF64 : IElement {
		std::function<void(TextInput&)> onUpdate;
		f64* value{ nullptr };
		Vec2 size{ 100,20 };
		Vec2 fontSize{ 10,20 };
		Vec4 color{ UNSET_COLOR };
		Vec4 colorFont{ UNSET_COLOR };
		Vec4 colorFontError{ UNSET_COLOR };
		BigTextureRef fonttexture{ "ConsolasSoft.png" };
	};
	namespace {
		struct _TextInputF64 : public TextInputF64 {
			_TextInputF64(TextInputF64&& e) : TextInputF64{ e } {}
			std::string str;
			f64 lastKnownValue{ 0.0f };
		};
	}
}

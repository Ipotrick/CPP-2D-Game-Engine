#pragma once

#include <string>
#include <string_view>
#include <optional>

#include <vector>
#include "RenderTypes.hpp"


class UIElement {
	std::string htmlData = "";
	std::vector<Drawable> htmlParsed;

	enum class State : char {
		Outside, 
		Head, ParseHeadType,
		Body, 
		Tail, 
		HeadOrTail,
	};

	struct ContextVariables {
		std::string_view componentName;
		Vec2 center;
		Vec2 size;
		Vec4 color;
	};

	const std::vector<std::string_view> componentNames = {
		"text",
		"div"
	};

	/**
		returns the component name, that matches the area in the htmlData from index begin to index end
		if it doesnt match anything, -1 is returned
	*/
	std::optional<std::string_view> matchComponentName(int begin);
	bool compareComponentNames(int begin, std::string_view name);
	bool indexInBounds(int index);
public:
	void set(std::string htmlData) {
		this->htmlData = htmlData;
	}
	std::vector<Drawable> get() {
		return this->htmlParsed;
	}
	bool compile();
};

inline bool isNameChar(char character) {
	return (character >= 'a' && character <= 'z')
		|| (character >= 'A' && character <= 'Z')
		|| (character >= '0' && character <= '9');
}

inline bool isLetter(char character) {
	return (character >= 'a' && character <= 'z')
		|| (character >= 'A' && character <= 'Z');
}
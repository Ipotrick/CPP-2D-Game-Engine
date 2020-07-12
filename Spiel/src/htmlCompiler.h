#pragma once

#include <String>
#include <vector>
#include "RenderTypes.h"

class UIElement {
	std::string htmlData = "";
	std::vector<Drawable> htmlParsed;
	enum class State : char {
		Outside, Head, Body, Tail, HeadOrTail,
	};
public:
	void set(std::string htmlData) {
		this->htmlData = htmlData;
	}
	std::vector<Drawable> get() {
		return this->htmlParsed;
	}
	bool compile();
};
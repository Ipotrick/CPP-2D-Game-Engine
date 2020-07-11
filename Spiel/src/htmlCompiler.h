#pragma once

#include <String>
#include <vector>
#include "RenderTypes.h"


class UIElement {
	std::string htmlData;
	std::vector<Drawable> htmlParsed;
	enum class State : char {
		Else, Head, Body, Tail
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


bool UIElement::compile() {
	State state = State::Else;
	bool error = false;
	for (auto& character : this->htmlData) {
		if (error) {
			break;
		}
		if (character == '\n'){
			continue;
		}
		switch (state) {
			case State::Else: {
				switch (character) {
					case '<': {
						state = State::Head;
						break;
					}
					case '>': {
						error = true;
						break;
					}
				}
				break;
			}
			case State::Head: {
				switch (character) {
					case '>': {
						state = State::Body;
						break;
					}
					case '<': {
						error = true;
						break;
					}
				}
				break;
			}
			case State::Body: {
				switch (character) {
					case '<': {
						state = State::Tail;
						break;
					}
					case '>': {
						error = true;
						break;
					}
				}
					break;
				}
			case State::Tail: {
				switch (character) {
					case '>': {
						state = State::Else;
						break;
					}
					case '<': {
						error = true;
						break;
					}
				}
				break;
			}
		}
	}
	return !error;
}
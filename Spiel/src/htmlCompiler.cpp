#include "htmlCompiler.h"

bool UIElement::compile() {
	State state = State::Outside;
	bool error = false;
	int verschachtelung = 0;
	for (auto& character : this->htmlData) {
		if (error) {
			break;
		}
		if (character == '\n') {
			continue;
		}
		switch (state) {
		case State::Outside: {
			switch (character) {
			case '<': {
				state = State::Head;
				verschachtelung++;
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
				state = State::HeadOrTail;
				break;
			}
			case '>': {
				error = true;
				break;
			}
			}
			break;
		}
		case State::HeadOrTail: {
			switch (character) {
			case '/': 
				state = State::Tail;
				verschachtelung--;
				break;
			default:
				state = State::Head;
				verschachtelung++;
				break;
			}
		}
		case State::Tail: {
			switch (character) {
			case '>': {
				if (verschachtelung > 0) {
					state = State::Body;
				}
				else {
					state = State::Outside;
				}
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
	if (state != State::Outside)
		error = true;
	if (verschachtelung != 0)
		error = true;
	return !error;
}
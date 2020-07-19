#include "htmlCompiler.h"

std::optional<std::string_view> UIElement::matchComponentName(int begin)
{
	std::vector<std::string_view> potentialMatches = this->componentNames;

	int index = 0;
	auto character = [&]() { return this->htmlData[index + begin];  };
	while (isNameChar(character())) {
		if (index + begin >= this->htmlData.size())
			return {};

		potentialMatches.erase(std::remove_if(potentialMatches.begin(), potentialMatches.end(),
			[&](std::string_view name) {
				return name[index] != character();
			}
		), potentialMatches.end());

		index++;
	}
	if (potentialMatches.size() == 0) {
		return {};
	}
	else {
		// return first match:
		return potentialMatches[0];
	}
}

bool UIElement::compareComponentNames(int begin, std::string_view name) {
	for (int i = 0; i < name.size(); i++) {
		if ((i + begin) >= this->htmlData.size())
			return false;
		if (name[i] != this->htmlData[i + begin])
			return false;
	}
	return true;
}

bool UIElement::indexInBounds(int index) {
	return index < this->htmlData.size();
}

bool UIElement::compile() {

	// parsing state:
	State state = State::Outside;
	int stateLength = 0;
	bool error = false;
	int stackIndex = 0;
	// ui context:
	std::vector<ContextVariables> contextStack;
	contextStack.push_back({ 
			std::string_view("Outside"),
			Vec2(),
			Vec2(),
			Vec4() 
		});


	auto changeState = [&](State newState) {
		state = newState;
		stateLength = -1;
	};

	for (int index = 0; index < this->htmlData.size(); index++) {
		// returns the charackter at the current location of the reading head (index)
		auto character = [&]() { return this->htmlData[index]; };

		if (character() == '\n' || character() == '\t') {
			continue;
		}
		if (error) {
			break;
		}

		switch (state) {
		case State::Outside:
			if (character() == '<') {
				changeState(State::HeadOrTail);
			}
			else if (character() != ' ') {
				error = true;
			}
			break;
		case State::ParseHeadType: {
			auto res = matchComponentName(index);
			if (res.has_value()) {
				auto name = res.value();
				index += name.size()-1;
				changeState(State::Head);
				contextStack.push_back({
					name,
					Vec2(),
					Vec2(),
					Vec4()
					});
			}
			else {
				error = true;
			}
			break;
		}
		case State::Head:
			if (character() == '>') {
				changeState(State::Body);
			}
			else if (character() != ' ') {
				error = true;
			}
			break;
		case State::Body:
			if (character() == '<') {
				changeState(State::HeadOrTail);
			} else if (!character() == ' ' && !isLetter(character())) {
				error = true;
			}
			break;
		case State::HeadOrTail:
			if (character() == '/') {
				changeState(State::Tail);
			}
			else {
				stackIndex++;
				changeState(State::ParseHeadType);
				index--;
			}
			break;
		case State::Tail:
			if (!compareComponentNames(index, contextStack[stackIndex].componentName)) {
				error = true;
			}
			else {
				index += contextStack[stackIndex].componentName.size();
				contextStack.pop_back();
				if (character() == '>') {
					stackIndex--;
					if (stackIndex == 0)
						changeState(State::Outside);
					else
						changeState(State::Body);
				}
				else {
					error = true;
				}
			}
			break;
		default:
			error = true;
		}
		stateLength++;	// count, how many characters the current state is containing
	}
	if (state != State::Outside)
		error = true;
	if (stackIndex != 0)
		error = true;
	return !error;
}
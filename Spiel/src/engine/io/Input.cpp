#include "Input.hpp"

std::optional<u8> keyToChar(Key key, bool shift)
{
	if (key <= Key::N_9 && key >= Key::NP_0) {
		return (u8)key;
	}
	else if (key <= Key::EQUAL && key >= Key::SPACE) {
		return (u8)key - (shift ? 0x10 : 0x00);
	}
	else if (key <= Key::Z && key >= Key::A) {
		return (u8)key + (shift ? 0x00 : 0x20);
	}
	else if (key <= Key::NP_9 && key >= Key::NP_0) {
		return (u8)key - (u8)Key::NP_0 + (u8)Key::N_0;
	}
	return {};
}

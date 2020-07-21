#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

#include "vector_math.hpp"

enum class Form : uint8_t {
	Circle = 0x00,
	Rectangle = 0x01
};

inline std::ostream& operator<<(std::ostream& ios, Form form) {
	ios << ((bool)form ? "Rectangle" : "Circle");
	return ios;
}

struct Vec2i {
	Vec2i(int x, int y) : x{x}, y{y} {}
	Vec2i() : x{ 0 }, y{ 0 } {}
	int x;
	int y;

	bool operator==(Vec2i const rhs) { return this->x == rhs.x && this->y == rhs.y; }
	bool operator!=(Vec2i const rhs) { return this->x != rhs.x || this->y != rhs.y; }
};

inline Vec2i operator+(Vec2i const a, Vec2i const b) {
	return { a.x + b.x, a.y + b.y };
}

template<typename T>
class Grid {
public:
	Grid(Vec2i size) : m_size{ size } {
		data.resize(m_size.x * m_size.y);
	}
	Grid(Vec2i size, T const& initVal) : m_size{ size } {
		data.resize(size.x * size.y, initVal);
	}
	inline T& at(int x, int y) {
		return data[x * m_size.y + y];
	}
	inline T& at(Vec2i pos) {
		return data[pos.x * m_size.y + pos.y];
	}
	inline bool isValid(int x, int y) const {
		return x > 0 && y > 0 && x < m_size.x && y < m_size.y;
	}
	inline bool isValid(Vec2i pos) const {
		return pos.x > 0 && pos.y > 0 && pos.x < m_size.x && pos.y < m_size.y;
	}
	inline Vec2i size() const {
		return Vec2i{ m_size.x, m_size.y };
	}
private:
	Vec2i m_size;
	std::vector<T> data;
};
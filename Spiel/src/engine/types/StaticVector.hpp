#pragma once

#include <cassert>
#include <array>
#include <cinttypes>

template<typename T, size_t CAPACITY>
class StaticVector {
public:

	void push_back(T const& el)
	{
		assert(m_size < CAPACITY);
		m_data[m_size++] = el;
	}

	void push_back(T && el)
	{
		assert(m_size < CAPACITY);
		m_data[m_size++] = std::move(el);
	}

	void pop_back()
	{
		assert(m_size > 0);
		m_size -= 1;
	}

	T& operator[](uint32_t index)
	{
		assert(index < m_size);
		return m_data[index];
	}

	T const& operator[](uint32_t index) const
	{
		assert(index < m_size);
		return m_data[index];
	}

	size_t size() const { return m_size; }
	constexpr size_t capacity() const { return CAPACITY; }

private:
	std::array<T, CAPACITY> m_data;
	size_t m_size{ 0 };
};
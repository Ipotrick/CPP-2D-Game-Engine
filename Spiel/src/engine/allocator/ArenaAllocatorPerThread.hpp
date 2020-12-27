#pragma once

#include <memory>
#include <iostream>
#include <mutex>
#include <thread>

#include "../types/ShortNames.hpp"

class PerThreadSharedArenaAllocatorStaticData {
private:
	template<typename T>
	friend class PerThreadSharedArenaAllocatorInterface;

	static inline std::mutex mut;

	static inline constexpr size_t MAX_SIZE{ 2 << 20 };	// 1 MB

	static inline u8 data[MAX_SIZE];
	static inline size_t currentEndOffset{ 0 };
	static inline size_t allocatedObjects{ 0 };
};

template<typename T>
struct PerThreadSharedArenaAllocatorInterface {
private:
	using Data = SharedArenaAllocatorStaticData;
public:

	/* Interface: */

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using propagate_on_container_move_assignment = std::true_type;

	constexpr value_type* allocate(size_type size)
	{
		std::unique_lock lock(Data::mut);
		const size_t OBJECT_SIZE = sizeof(value_type) * size;

		if (Data::currentEndOffset + OBJECT_SIZE > Data::MAX_SIZE) {
			throw new std::bad_alloc();
		}

		u8* ptr = Data::data + Data::currentEndOffset;
		Data::currentEndOffset += OBJECT_SIZE;
		Data::allocatedObjects += 1;
		return reinterpret_cast<value_type*>(ptr);
	}

	constexpr void deallocate(value_type* p, size_type n)
	{
		std::unique_lock lock(Data::mut);
		Data::allocatedObjects -= 1;
		if (Data::allocatedObjects == 0) {
			Data::currentEndOffset = 0;
		}
	}

	constexpr bool operator==(PerThreadSharedArenaAllocatorInterface<T> const& rhs) const { return true; }

	constexpr size_t allocatedObjects() const { return Data::allocatedObjects; }
};
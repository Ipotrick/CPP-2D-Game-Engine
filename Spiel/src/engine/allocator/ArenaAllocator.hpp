#pragma once

#include "../types/ShortNames.hpp"
#include "../types/Spinlock.hpp"

template<typename T>
class SharedArenaAllocatorInterface {
public:
	/* Interface: */

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using propagate_on_container_move_assignment = std::true_type;

	constexpr value_type* allocate(size_type size)
	{
		Spinlock lock(atomLock);
		const size_t OBJECT_SIZE = sizeof(value_type) * size;

		if (currentEndOffset + OBJECT_SIZE > MAX_SIZE) {
			throw new std::bad_alloc();
		}

		u8* ptr = data + currentEndOffset;
		currentEndOffset += OBJECT_SIZE;
		allocatedObjects += 1;
		return reinterpret_cast<value_type*>(ptr);
	}

	constexpr void deallocate(value_type* p, size_type n)
	{
		Spinlock lock(atomLock);
		allocatedObjects -= 1;
		if (allocatedObjects == 0) {
			currentEndOffset = 0;
		}
	}

	constexpr bool operator==(SharedArenaAllocatorInterface<T> const& rhs) const { return true; }

	constexpr size_t allocObjectCount() const { return allocatedObjects; }
private:

	static inline std::atomic_flag atomLock = {};

	static inline constexpr size_t MAX_SIZE{ 2 << 24 };	// 16 MB

	static inline u8 data[MAX_SIZE];
	static inline size_t currentEndOffset{ 0 };
	static inline size_t allocatedObjects{ 0 };
};
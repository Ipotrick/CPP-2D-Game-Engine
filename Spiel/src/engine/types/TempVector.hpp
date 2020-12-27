#pragma once

#include <vector>

#include "../allocator/ArenaAllocator.hpp"

template<typename T>
using TempVector = std::vector<T, SharedArenaAllocatorInterface<T>>;
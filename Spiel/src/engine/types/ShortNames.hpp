#pragma once

#include <cinttypes>

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;
using f80 = long double;

/**
 * static_cast with shortened name.
 *
 * \param value value to cast to T
 * \param T type to cast to
 * \return value castet to T
 */
template<typename T>
inline constexpr T cast(auto value) { return static_cast<T>(value); }

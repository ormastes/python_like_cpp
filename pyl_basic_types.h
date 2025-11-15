#pragma once

#include <cstdint>
#include <cstddef>

namespace pyl {

// =======================================================
//  Short aliases for standard integer / float / size types
// =======================================================

// Unsigned integers
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
#ifdef __SIZEOF_INT128__
using u128 = unsigned __int128;
#endif

// Signed integers
using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
#ifdef __SIZEOF_INT128__
using i128 = __int128;
#endif

// Floating point
using f32  = float;
using f64  = double;
using f128 = long double; // not guaranteed to be true 128-bit, but widest FP type

// Size / index types
using usize = std::size_t;
using isize = std::ptrdiff_t;

} // namespace pyl

// ========================================
//  User-defined literals for these aliases
// ========================================
//
//  Integer examples:
//      123_u32
//      5_u8
//      0_u64
//      42_u16
//      -1i8   (unary - applied to literal 1i8)
//
//  Floating examples:
//      1.0_f32
//      2.5_f64
//      3.0_f128
//
// Note: literals are in the global namespace for direct use.

//
// Unsigned integer literals:  _u8, _u16, _u32, _u64, (_u128)
//

constexpr pyl::u8 operator""_u8(unsigned long long v) noexcept {
    return static_cast<pyl::u8>(v);
}

constexpr pyl::u16 operator""_u16(unsigned long long v) noexcept {
    return static_cast<pyl::u16>(v);
}

constexpr pyl::u32 operator""_u32(unsigned long long v) noexcept {
    return static_cast<pyl::u32>(v);
}

constexpr pyl::u64 operator""_u64(unsigned long long v) noexcept {
    return static_cast<pyl::u64>(v);
}

#ifdef __SIZEOF_INT128__
constexpr pyl::u128 operator""_u128(unsigned long long v) noexcept {
    return static_cast<pyl::u128>(v);
}
#endif

//
// Signed integer literals:  _i8, _i16, _i32, _i64, (_i128)
//
// Usage:
//    -1_i8   // parsed as unary minus applied to literal 1_i8
//    10_i32
//

constexpr pyl::i8 operator""_i8(unsigned long long v) noexcept {
    return static_cast<pyl::i8>(v);
}

constexpr pyl::i16 operator""_i16(unsigned long long v) noexcept {
    return static_cast<pyl::i16>(v);
}

constexpr pyl::i32 operator""_i32(unsigned long long v) noexcept {
    return static_cast<pyl::i32>(v);
}

constexpr pyl::i64 operator""_i64(unsigned long long v) noexcept {
    return static_cast<pyl::i64>(v);
}

#ifdef __SIZEOF_INT128__
constexpr pyl::i128 operator""_i128(unsigned long long v) noexcept {
    return static_cast<pyl::i128>(v);
}
#endif

//
// Floating-point literals:  _f32, _f64, _f128
//
// Usage:
//    1.0_f32
//    2.5_f64
//    3.141592653589793_f64
//    1.0_f128
//

constexpr pyl::f32 operator""_f32(long double v) noexcept {
    return static_cast<pyl::f32>(v);
}

constexpr pyl::f64 operator""_f64(long double v) noexcept {
    return static_cast<pyl::f64>(v);
}

constexpr pyl::f128 operator""_f128(long double v) noexcept {
    return static_cast<pyl::f128>(v);
}

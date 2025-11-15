#include <catch2/catch_test_macros.hpp>
#include "pyl_basic_types.h"
#include <type_traits>

using namespace pyl;

// ========================================
// Type alias tests
// ========================================

TEST_CASE("u8 is uint8_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<u8, std::uint8_t>);
    REQUIRE(sizeof(u8) == 1);
}

TEST_CASE("u16 is uint16_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<u16, std::uint16_t>);
    REQUIRE(sizeof(u16) == 2);
}

TEST_CASE("u32 is uint32_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<u32, std::uint32_t>);
    REQUIRE(sizeof(u32) == 4);
}

TEST_CASE("u64 is uint64_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<u64, std::uint64_t>);
    REQUIRE(sizeof(u64) == 8);
}

#ifdef __SIZEOF_INT128__
TEST_CASE("u128 is unsigned __int128", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<u128, unsigned __int128>);
    REQUIRE(sizeof(u128) == 16);
}
#endif

TEST_CASE("i8 is int8_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<i8, std::int8_t>);
    REQUIRE(sizeof(i8) == 1);
}

TEST_CASE("i16 is int16_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<i16, std::int16_t>);
    REQUIRE(sizeof(i16) == 2);
}

TEST_CASE("i32 is int32_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<i32, std::int32_t>);
    REQUIRE(sizeof(i32) == 4);
}

TEST_CASE("i64 is int64_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<i64, std::int64_t>);
    REQUIRE(sizeof(i64) == 8);
}

#ifdef __SIZEOF_INT128__
TEST_CASE("i128 is __int128", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<i128, __int128>);
    REQUIRE(sizeof(i128) == 16);
}
#endif

TEST_CASE("f32 is float", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<f32, float>);
    REQUIRE(sizeof(f32) == 4);
}

TEST_CASE("f64 is double", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<f64, double>);
    REQUIRE(sizeof(f64) == 8);
}

TEST_CASE("f128 is long double", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<f128, long double>);
    REQUIRE(sizeof(f128) >= 8);
}

TEST_CASE("usize is size_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<usize, std::size_t>);
}

TEST_CASE("isize is ptrdiff_t", "[pyl_basic_types]") {
    REQUIRE(std::is_same_v<isize, std::ptrdiff_t>);
}

// ========================================
// User-defined literal tests - unsigned
// ========================================

TEST_CASE("_u8 literal creates u8", "[pyl_basic_types]") {
    auto a = 42_u8;
    REQUIRE(std::is_same_v<decltype(a), u8>);
    REQUIRE(a == 42);
}

TEST_CASE("_u16 literal creates u16", "[pyl_basic_types]") {
    auto a = 1234_u16;
    REQUIRE(std::is_same_v<decltype(a), u16>);
    REQUIRE(a == 1234);
}

TEST_CASE("_u32 literal creates u32", "[pyl_basic_types]") {
    auto a = 123456_u32;
    REQUIRE(std::is_same_v<decltype(a), u32>);
    REQUIRE(a == 123456);
}

TEST_CASE("_u64 literal creates u64", "[pyl_basic_types]") {
    auto a = 123456789_u64;
    REQUIRE(std::is_same_v<decltype(a), u64>);
    REQUIRE(a == 123456789);
}

#ifdef __SIZEOF_INT128__
TEST_CASE("_u128 literal creates u128", "[pyl_basic_types]") {
    auto a = 42_u128;
    REQUIRE(std::is_same_v<decltype(a), u128>);
    REQUIRE(a == 42);
}
#endif

// ========================================
// User-defined literal tests - signed
// ========================================

TEST_CASE("i8 literal creates i8", "[pyl_basic_types]") {
    auto a = 42_i8;
    REQUIRE(std::is_same_v<decltype(a), i8>);
    REQUIRE(a == 42);
}

TEST_CASE("i8 literal with unary minus", "[pyl_basic_types]") {
    auto a = -1_i8;
    // Note: unary minus promotes the type, so we need to explicitly cast
    i8 b = -1_i8;
    REQUIRE(b == -1);
    REQUIRE(a == -1);
}

TEST_CASE("i16 literal creates i16", "[pyl_basic_types]") {
    auto a = 1000_i16;
    REQUIRE(std::is_same_v<decltype(a), i16>);
    REQUIRE(a == 1000);
}

TEST_CASE("i16 literal with unary minus", "[pyl_basic_types]") {
    auto a = -1000_i16;
    // Note: unary minus promotes the type, so we need to explicitly cast
    i16 b = -1000_i16;
    REQUIRE(b == -1000);
    REQUIRE(a == -1000);
}

TEST_CASE("i32 literal creates i32", "[pyl_basic_types]") {
    auto a = 100000_i32;
    REQUIRE(std::is_same_v<decltype(a), i32>);
    REQUIRE(a == 100000);
}

TEST_CASE("i32 literal with unary minus", "[pyl_basic_types]") {
    auto a = -100000_i32;
    REQUIRE(std::is_same_v<decltype(a), i32>);
    REQUIRE(a == -100000);
}

TEST_CASE("i64 literal creates i64", "[pyl_basic_types]") {
    auto a = 1000000000_i64;
    REQUIRE(std::is_same_v<decltype(a), i64>);
    REQUIRE(a == 1000000000);
}

TEST_CASE("i64 literal with unary minus", "[pyl_basic_types]") {
    auto a = -1000000000_i64;
    REQUIRE(std::is_same_v<decltype(a), i64>);
    REQUIRE(a == -1000000000);
}

#ifdef __SIZEOF_INT128__
TEST_CASE("i128 literal creates i128", "[pyl_basic_types]") {
    auto a = 42_i128;
    REQUIRE(std::is_same_v<decltype(a), i128>);
    REQUIRE(a == 42);
}

TEST_CASE("i128 literal with unary minus", "[pyl_basic_types]") {
    auto a = -42_i128;
    REQUIRE(std::is_same_v<decltype(a), i128>);
    REQUIRE(a == -42);
}
#endif

// ========================================
// User-defined literal tests - floating point
// ========================================

TEST_CASE("_f32 literal creates f32", "[pyl_basic_types]") {
    auto a = 1.5_f32;
    REQUIRE(std::is_same_v<decltype(a), f32>);
    REQUIRE(a == 1.5f);
}

TEST_CASE("_f32 literal with negative", "[pyl_basic_types]") {
    auto a = -2.5_f32;
    REQUIRE(std::is_same_v<decltype(a), f32>);
    REQUIRE(a == -2.5f);
}

TEST_CASE("_f64 literal creates f64", "[pyl_basic_types]") {
    auto a = 3.14159_f64;
    REQUIRE(std::is_same_v<decltype(a), f64>);
    REQUIRE(a == 3.14159);
}

TEST_CASE("_f64 literal with negative", "[pyl_basic_types]") {
    auto a = -1.0_f64;
    REQUIRE(std::is_same_v<decltype(a), f64>);
    REQUIRE(a == -1.0);
}

TEST_CASE("_f128 literal creates f128", "[pyl_basic_types]") {
    auto a = 1.0_f128;
    REQUIRE(std::is_same_v<decltype(a), f128>);
    REQUIRE(a == 1.0L);
}

TEST_CASE("_f128 literal with negative", "[pyl_basic_types]") {
    auto a = -3.0_f128;
    REQUIRE(std::is_same_v<decltype(a), f128>);
    REQUIRE(a == -3.0L);
}

// ========================================
// Constexpr tests
// ========================================

TEST_CASE("unsigned literals are constexpr", "[pyl_basic_types]") {
    constexpr auto a = 100_u32;
    constexpr auto b = 5_u8;
    constexpr auto c = 1000_u16;
    constexpr auto d = 1_u64;

    static_assert(a == 100u, "u32 literal is constexpr");
    static_assert(b == 5u, "u8 literal is constexpr");
    static_assert(c == 1000u, "u16 literal is constexpr");
    static_assert(d == 1u, "u64 literal is constexpr");

    REQUIRE(a == 100);
    REQUIRE(b == 5);
    REQUIRE(c == 1000);
    REQUIRE(d == 1);
}

TEST_CASE("signed literals are constexpr", "[pyl_basic_types]") {
    constexpr auto a = 100_i32;
    constexpr auto b = -1_i8;
    constexpr auto c = 1000_i16;
    constexpr auto d = -1000_i64;

    static_assert(a == 100, "i32 literal is constexpr");
    static_assert(b == -1, "i8 literal is constexpr");
    static_assert(c == 1000, "i16 literal is constexpr");
    static_assert(d == -1000, "i64 literal is constexpr");

    REQUIRE(a == 100);
    REQUIRE(b == -1);
    REQUIRE(c == 1000);
    REQUIRE(d == -1000);
}

TEST_CASE("floating literals are constexpr", "[pyl_basic_types]") {
    constexpr auto a = 1.0_f32;
    constexpr auto b = 2.5_f64;
    constexpr auto c = 3.0_f128;

    static_assert(a == 1.0f, "f32 literal is constexpr");
    static_assert(b == 2.5, "f64 literal is constexpr");
    static_assert(c == 3.0L, "f128 literal is constexpr");

    REQUIRE(a == 1.0f);
    REQUIRE(b == 2.5);
    REQUIRE(c == 3.0L);
}

// ========================================
// Arithmetic operations tests
// ========================================

TEST_CASE("u32 arithmetic operations", "[pyl_basic_types]") {
    u32 a = 100_u32;
    u32 b = 50_u32;

    REQUIRE(a + b == 150);
    REQUIRE(a - b == 50);
    REQUIRE(a * b == 5000);
    REQUIRE(a / b == 2);
}

TEST_CASE("i32 arithmetic operations", "[pyl_basic_types]") {
    i32 a = 100_i32;
    i32 b = -50_i32;

    REQUIRE(a + b == 50);
    REQUIRE(a - b == 150);
    REQUIRE(a * b == -5000);
    REQUIRE(a / b == -2);
}

TEST_CASE("f64 arithmetic operations", "[pyl_basic_types]") {
    f64 a = 10.0_f64;
    f64 b = 2.5_f64;

    REQUIRE(a + b == 12.5);
    REQUIRE(a - b == 7.5);
    REQUIRE(a * b == 25.0);
    REQUIRE(a / b == 4.0);
}

// ========================================
// Comparison tests
// ========================================

TEST_CASE("u32 comparisons", "[pyl_basic_types]") {
    u32 a = 100_u32;
    u32 b = 50_u32;

    REQUIRE(a > b);
    REQUIRE(b < a);
    REQUIRE(a >= b);
    REQUIRE(b <= a);
    REQUIRE(a != b);
    REQUIRE(a == 100_u32);
}

TEST_CASE("i32 comparisons", "[pyl_basic_types]") {
    i32 a = -10_i32;
    i32 b = -20_i32;

    REQUIRE(a > b);
    REQUIRE(b < a);
    REQUIRE(a >= b);
    REQUIRE(b <= a);
    REQUIRE(a != b);
    REQUIRE(a == -10_i32);
}

TEST_CASE("f64 comparisons", "[pyl_basic_types]") {
    f64 a = 3.14_f64;
    f64 b = 2.71_f64;

    REQUIRE(a > b);
    REQUIRE(b < a);
    REQUIRE(a >= b);
    REQUIRE(b <= a);
    REQUIRE(a != b);
    REQUIRE(a == 3.14_f64);
}

// ========================================
// Type signedness tests
// ========================================

TEST_CASE("unsigned types are unsigned", "[pyl_basic_types]") {
    REQUIRE(std::is_unsigned_v<u8>);
    REQUIRE(std::is_unsigned_v<u16>);
    REQUIRE(std::is_unsigned_v<u32>);
    REQUIRE(std::is_unsigned_v<u64>);
}

TEST_CASE("signed types are signed", "[pyl_basic_types]") {
    REQUIRE(std::is_signed_v<i8>);
    REQUIRE(std::is_signed_v<i16>);
    REQUIRE(std::is_signed_v<i32>);
    REQUIRE(std::is_signed_v<i64>);
}

TEST_CASE("floating types are signed", "[pyl_basic_types]") {
    REQUIRE(std::is_signed_v<f32>);
    REQUIRE(std::is_signed_v<f64>);
    REQUIRE(std::is_signed_v<f128>);
}

// ========================================
// Mixed type operations
// ========================================

TEST_CASE("can assign literals to variables", "[pyl_basic_types]") {
    u32 a = 123_u32;
    i32 b = -456_i32;
    f64 c = 7.89_f64;

    REQUIRE(a == 123);
    REQUIRE(b == -456);
    REQUIRE(c == 7.89);
}

TEST_CASE("can use in function parameters", "[pyl_basic_types]") {
    auto add_u32 = [](u32 a, u32 b) { return a + b; };
    auto add_i32 = [](i32 a, i32 b) { return a + b; };
    auto add_f64 = [](f64 a, f64 b) { return a + b; };

    REQUIRE(add_u32(10_u32, 20_u32) == 30);
    REQUIRE(add_i32(10_i32, -5_i32) == 5);
    REQUIRE(add_f64(1.5_f64, 2.5_f64) == 4.0);
}

TEST_CASE("usize and isize work correctly", "[pyl_basic_types]") {
    usize array_size = 10;
    isize offset = -5;

    REQUIRE(array_size == 10);
    REQUIRE(offset == -5);

    // Common use cases
    usize index = 0;
    index += 5;
    REQUIRE(index == 5);

    isize diff = static_cast<isize>(array_size) + offset;
    REQUIRE(diff == 5);
}

#include <catch2/catch_test_macros.hpp>
#include "pyl_strong_num.h"
#include <sstream>

using namespace pyl;

// Define some strong numeric types using the macro
STRONG_NUM(UserId);  // Default: Int, 32 bits -> int32_t

// For types with enum parameters, define them manually to avoid macro issues with ::
struct CountTag {};
using Count = StrongNumber<NumTypeT<NumKind::Uint, NumBits::B32>, CountTag>;

struct PriceTag {};
using Price = StrongNumber<NumTypeT<NumKind::Float, NumBits::B64>, PriceTag>;

struct Length16Tag {};
using Length16 = StrongNumber<NumTypeT<NumKind::Size, NumBits::B16>, Length16Tag>;

// Example with multiple strong types sharing same Tag to demo widening
struct DistanceTag {};
using DistanceI32 = StrongNumber<std::int32_t, DistanceTag>;
using DistanceI64 = StrongNumber<std::int64_t, DistanceTag>;
using DistanceU32 = StrongNumber<std::uint32_t, DistanceTag>;
using DistanceU16 = StrongNumber<std::uint16_t, DistanceTag>;
using DistanceF32 = StrongNumber<float, DistanceTag>;
using DistanceF64 = StrongNumber<double, DistanceTag>;

TEST_CASE("STRONG_NUM basic construction with default Int32", "[pyl_strong_num]") {
    UserId u{42};
    REQUIRE(u.value() == 42);
}

TEST_CASE("STRONG_NUM with Uint kind", "[pyl_strong_num]") {
    Count c{100u};
    REQUIRE(c.value() == 100u);
}

TEST_CASE("STRONG_NUM with Float kind and 64 bits", "[pyl_strong_num]") {
    Price p{19.99};
    REQUIRE(p.value() == 19.99);
}

TEST_CASE("STRONG_NUM with Size kind and 16 bits", "[pyl_strong_num]") {
    Length16 len{123};
    REQUIRE(len.value() == 123);
}

TEST_CASE("StrongNumber arithmetic - same type", "[pyl_strong_num]") {
    UserId u1{42};
    UserId u2{100};

    auto u_sum = u1 + u2;
    REQUIRE(u_sum.value() == 142);

    auto u_diff = u2 - u1;
    REQUIRE(u_diff.value() == 58);

    auto u_prod = UserId{3} * UserId{4};
    REQUIRE(u_prod.value() == 12);

    auto u_quot = UserId{20} / UserId{5};
    REQUIRE(u_quot.value() == 4);
}

TEST_CASE("StrongNumber arithmetic - different widths same tag", "[pyl_strong_num]") {
    DistanceI32 d32{100};
    DistanceI64 d64{200};

    // Common type will be int64_t
    auto sum = d32 + d64;
    REQUIRE(sum.value() == 300);
}

TEST_CASE("StrongNumber unary operators", "[pyl_strong_num]") {
    UserId u{42};

    auto pos = +u;
    REQUIRE(pos.value() == 42);

    auto neg = -u;
    REQUIRE(neg.value() == -42);
}

TEST_CASE("StrongNumber compound assignment operators", "[pyl_strong_num]") {
    UserId u{10};

    u += UserId{5};
    REQUIRE(u.value() == 15);

    u -= UserId{3};
    REQUIRE(u.value() == 12);

    u *= UserId{2};
    REQUIRE(u.value() == 24);

    u /= UserId{4};
    REQUIRE(u.value() == 6);
}

TEST_CASE("StrongNumber comparisons - same type", "[pyl_strong_num]") {
    UserId u1{42};
    UserId u2{100};
    UserId u3{42};

    REQUIRE(u1 == u3);
    REQUIRE(u1 != u2);
    REQUIRE(u1 < u2);
    REQUIRE(u2 > u1);
    REQUIRE(u1 <= u3);
    REQUIRE(u1 <= u2);
    REQUIRE(u2 >= u1);
    REQUIRE(u3 >= u1);
}

TEST_CASE("StrongNumber comparisons - different widths same tag", "[pyl_strong_num]") {
    DistanceI32 d32{100};
    DistanceI64 d64{100};

    REQUIRE(d32 == d64);
    REQUIRE_FALSE(d32 != d64);
}

TEST_CASE("StrongNumber implicit widening - int32 to int64", "[pyl_strong_num]") {
    DistanceI32 d32{100};
    DistanceI64 d64 = d32; // implicit widening

    REQUIRE(d64.value() == 100);
}

TEST_CASE("StrongNumber implicit widening - float to double", "[pyl_strong_num]") {
    DistanceF32 df32{1.5f};
    DistanceF64 df64 = df32; // implicit widening

    REQUIRE(df64.value() == 1.5);
}

TEST_CASE("StrongNumber explicit narrowing - int64 to int32", "[pyl_strong_num]") {
    DistanceI64 d64{100};
    DistanceI32 d32 = static_cast<DistanceI32>(d64); // explicit narrowing

    REQUIRE(d32.value() == 100);
}

TEST_CASE("StrongNumber explicit narrowing - uint32 to uint16", "[pyl_strong_num]") {
    DistanceU32 du32{100};
    DistanceU16 du16 = static_cast<DistanceU16>(du32); // explicit narrowing

    REQUIRE(du16.value() == 100);
}

TEST_CASE("StrongNumber explicit cross-category - int to float", "[pyl_strong_num]") {
    DistanceI32 di32{100};
    DistanceF32 df32 = static_cast<DistanceF32>(di32); // explicit: different category

    REQUIRE(df32.value() == 100.0f);
}

TEST_CASE("StrongNumber explicit conversion to primitive int", "[pyl_strong_num]") {
    UserId u{42};
    std::int32_t raw = static_cast<std::int32_t>(u);

    REQUIRE(raw == 42);
}

TEST_CASE("StrongNumber explicit conversion to primitive float", "[pyl_strong_num]") {
    Price p{19.99};
    double raw = static_cast<double>(p);

    REQUIRE(raw == 19.99);
}

TEST_CASE("StrongNumber swap", "[pyl_strong_num]") {
    UserId u1{42};
    UserId u2{100};

    swap(u1, u2);

    REQUIRE(u1.value() == 100);
    REQUIRE(u2.value() == 42);
}

TEST_CASE("StrongNumber ostream output", "[pyl_strong_num]") {
    UserId u{42};
    std::ostringstream oss;
    oss << u;

    REQUIRE(oss.str() == "42");
}

TEST_CASE("StrongNumber constexpr construction", "[pyl_strong_num]") {
    constexpr UserId cu{7};
    static_assert(cu.value() == 7, "constexpr construction works");
    REQUIRE(cu.value() == 7);
}

TEST_CASE("StrongNumber constexpr arithmetic", "[pyl_strong_num]") {
    constexpr UserId u1{10};
    constexpr UserId u2{5};
    constexpr auto sum = u1 + u2;
    static_assert(sum.value() == 15, "constexpr arithmetic works");
    REQUIRE(sum.value() == 15);
}

TEST_CASE("StrongNumber constexpr comparisons", "[pyl_strong_num]") {
    constexpr UserId u1{42};
    constexpr UserId u2{42};
    static_assert(u1 == u2, "constexpr equality works");
    REQUIRE(u1 == u2);
}

TEST_CASE("StrongNumber constexpr widening conversion", "[pyl_strong_num]") {
    constexpr DistanceI32 d32{5};
    constexpr DistanceI64 d64 = d32; // implicit widening in constexpr
    static_assert(d64.value() == 5, "constexpr widening works");
    REQUIRE(d64.value() == 5);
}

TEST_CASE("StrongNumber constexpr explicit conversion to primitive", "[pyl_strong_num]") {
    constexpr UserId cu{7};
    constexpr std::int32_t cu_raw = static_cast<std::int32_t>(cu);
    static_assert(cu_raw == 7, "constexpr conversion works");
    REQUIRE(cu_raw == 7);
}

TEST_CASE("NumType mapping - Uint types", "[pyl_strong_num]") {
    REQUIRE(sizeof(NumTypeT<NumKind::Uint, NumBits::B8>) == 1);
    REQUIRE(sizeof(NumTypeT<NumKind::Uint, NumBits::B16>) == 2);
    REQUIRE(sizeof(NumTypeT<NumKind::Uint, NumBits::B32>) == 4);
    REQUIRE(sizeof(NumTypeT<NumKind::Uint, NumBits::B64>) == 8);
}

TEST_CASE("NumType mapping - Int types", "[pyl_strong_num]") {
    REQUIRE(sizeof(NumTypeT<NumKind::Int, NumBits::B8>) == 1);
    REQUIRE(sizeof(NumTypeT<NumKind::Int, NumBits::B16>) == 2);
    REQUIRE(sizeof(NumTypeT<NumKind::Int, NumBits::B32>) == 4);
    REQUIRE(sizeof(NumTypeT<NumKind::Int, NumBits::B64>) == 8);
}

TEST_CASE("NumType mapping - Float types", "[pyl_strong_num]") {
    REQUIRE(sizeof(NumTypeT<NumKind::Float, NumBits::B32>) == 4);
    REQUIRE(sizeof(NumTypeT<NumKind::Float, NumBits::B64>) == 8);
    REQUIRE(sizeof(NumTypeT<NumKind::Float, NumBits::B128>) >= 8); // long double is at least 8 bytes
}

TEST_CASE("NumType mapping - Size types", "[pyl_strong_num]") {
    REQUIRE(sizeof(NumTypeT<NumKind::Size, NumBits::B8>) == 1);
    REQUIRE(sizeof(NumTypeT<NumKind::Size, NumBits::B16>) == 2);
    REQUIRE(sizeof(NumTypeT<NumKind::Size, NumBits::B32>) == 4);
    REQUIRE(sizeof(NumTypeT<NumKind::Size, NumBits::B64>) == 8);
}

TEST_CASE("is_widening_type - same type", "[pyl_strong_num]") {
    REQUIRE(strong_num_detail::is_widening_type_v<int32_t, int32_t>);
    REQUIRE(strong_num_detail::is_widening_type_v<float, float>);
}

TEST_CASE("is_widening_type - integral widening", "[pyl_strong_num]") {
    REQUIRE(strong_num_detail::is_widening_type_v<int32_t, int64_t>);
    REQUIRE(strong_num_detail::is_widening_type_v<uint16_t, uint32_t>);
    REQUIRE_FALSE(strong_num_detail::is_widening_type_v<int64_t, int32_t>);
}

TEST_CASE("is_widening_type - float widening", "[pyl_strong_num]") {
    REQUIRE(strong_num_detail::is_widening_type_v<float, double>);
    REQUIRE_FALSE(strong_num_detail::is_widening_type_v<double, float>);
}

TEST_CASE("is_widening_type - cross category is not widening", "[pyl_strong_num]") {
    REQUIRE_FALSE(strong_num_detail::is_widening_type_v<int32_t, uint32_t>);
    REQUIRE_FALSE(strong_num_detail::is_widening_type_v<int32_t, float>);
    REQUIRE_FALSE(strong_num_detail::is_widening_type_v<float, int32_t>);
}

TEST_CASE("StrongNumber default construction", "[pyl_strong_num]") {
    UserId u;
    REQUIRE(u.value() == 0);

    Price p;
    REQUIRE(p.value() == 0.0);
}

TEST_CASE("StrongNumber multiple types interaction", "[pyl_strong_num]") {
    UserId uid{1};
    Count cnt{10u};

    // These have different tags, so they cannot be directly added
    // This test verifies they are distinct types
    REQUIRE(uid.value() == 1);
    REQUIRE(cnt.value() == 10u);

    // Verify they have different tags by checking they're not the same type
    REQUIRE_FALSE((std::is_same_v<UserId, Count>));
}

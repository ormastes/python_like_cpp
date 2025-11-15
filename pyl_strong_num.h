#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <ostream>

namespace pyl {

// =========================
// 1. Kind / Bits enums
// =========================

enum class NumKind {
    Uint,
    Int,
    Float,
    Size,
};

enum class NumBits {
    B8   = 8,
    B16  = 16,
    B32  = 32,
    B64  = 64,
    B128 = 128,
};

// =========================
// 2. Kind+Bits -> C++ type
// =========================

template <NumKind K, NumBits B>
struct NumType; // primary template, only specialized where valid

// Unsigned integers
template <> struct NumType<NumKind::Uint, NumBits::B8>  { using type = std::uint8_t;  };
template <> struct NumType<NumKind::Uint, NumBits::B16> { using type = std::uint16_t; };
template <> struct NumType<NumKind::Uint, NumBits::B32> { using type = std::uint32_t; };
template <> struct NumType<NumKind::Uint, NumBits::B64> { using type = std::uint64_t; };
#ifdef __SIZEOF_INT128__
template <> struct NumType<NumKind::Uint, NumBits::B128> { using type = unsigned __int128; };
#endif

// Signed integers
template <> struct NumType<NumKind::Int, NumBits::B8>  { using type = std::int8_t;  };
template <> struct NumType<NumKind::Int, NumBits::B16> { using type = std::int16_t; };
template <> struct NumType<NumKind::Int, NumBits::B32> { using type = std::int32_t; };
template <> struct NumType<NumKind::Int, NumBits::B64> { using type = std::int64_t; };
#ifdef __SIZEOF_INT128__
template <> struct NumType<NumKind::Int, NumBits::B128> { using type = __int128; };
#endif

// Floats
template <> struct NumType<NumKind::Float, NumBits::B32> { using type = float;       };
template <> struct NumType<NumKind::Float, NumBits::B64> { using type = double;      };
template <> struct NumType<NumKind::Float, NumBits::B128>{ using type = long double; };

// Sizes: width-based unsigned mapping
template <> struct NumType<NumKind::Size, NumBits::B8>  { using type = std::uint8_t;  };
template <> struct NumType<NumKind::Size, NumBits::B16> { using type = std::uint16_t; };
template <> struct NumType<NumKind::Size, NumBits::B32> { using type = std::uint32_t; };
template <> struct NumType<NumKind::Size, NumBits::B64> { using type = std::uint64_t; };
#ifdef __SIZEOF_INT128__
template <> struct NumType<NumKind::Size, NumBits::B128>{ using type = unsigned __int128; };
#endif

template <NumKind K, NumBits B>
using NumTypeT = typename NumType<K, B>::type;

// =========================
// 3. Widening trait
// =========================

namespace strong_num_detail {

template <typename From, typename To>
struct is_widening_type {
    static constexpr bool value = [] {
        // same type
        if constexpr (std::is_same_v<From, To>) {
            return true;
        }
        // integral -> integral, same signedness, sizeof(To) >= sizeof(From)
        else if constexpr (std::is_integral_v<From> && std::is_integral_v<To> &&
                           std::is_signed_v<From> == std::is_signed_v<To>) {
            return sizeof(To) >= sizeof(From);
        }
        // float -> float, sizeof(To) >= sizeof(From)
        else if constexpr (std::is_floating_point_v<From> &&
                           std::is_floating_point_v<To>) {
            return sizeof(To) >= sizeof(From);
        }
        // everything else (int <-> uint, int <-> float, etc.) is NOT widening
        else {
            return false;
        }
    }();
};

template <typename From, typename To>
inline constexpr bool is_widening_type_v = is_widening_type<From, To>::value;

} // namespace strong_num_detail

// =========================
// 4. StrongNumber<T, Tag>
// =========================

template <typename T, typename Tag>
class StrongNumber {
    static_assert(std::is_arithmetic_v<T>,
                  "StrongNumber<T, Tag> requires arithmetic T");

public:
    using value_type = T;
    using tag_type   = Tag;

    // constructors
    constexpr StrongNumber() = default;
    explicit constexpr StrongNumber(T v) : value_(v) {}

    // access
    constexpr T value() const noexcept { return value_; }

    // arithmetic (same T, same Tag)
    constexpr StrongNumber operator+() const noexcept { return *this; }
    constexpr StrongNumber operator-() const noexcept { return StrongNumber(-value_); }

    constexpr StrongNumber& operator+=(const StrongNumber& other) noexcept {
        value_ += other.value_;
        return *this;
    }
    constexpr StrongNumber& operator-=(const StrongNumber& other) noexcept {
        value_ -= other.value_;
        return *this;
    }
    constexpr StrongNumber& operator*=(const StrongNumber& other) noexcept {
        value_ *= other.value_;
        return *this;
    }
    constexpr StrongNumber& operator/=(const StrongNumber& other) {
        value_ /= other.value_;
        return *this;
    }

    friend constexpr void swap(StrongNumber& a, StrongNumber& b) noexcept {
        auto tmp = a.value_;
        a.value_ = b.value_;
        b.value_ = tmp;
    }

    // ------------------------
    // Conversions
    // ------------------------

    // (A) IMPLICIT: widening StrongNumber<T,Tag> -> StrongNumber<U,Tag>
    template <typename U,
              std::enable_if_t<
                  strong_num_detail::is_widening_type_v<T, U>, int> = 0>
    constexpr operator StrongNumber<U, Tag>() const noexcept {
        return StrongNumber<U, Tag>(static_cast<U>(value_));
    }

    // (B) EXPLICIT: non-widening StrongNumber<T,Tag> -> StrongNumber<U,Tag>
    template <typename U,
              std::enable_if_t<
                  !strong_num_detail::is_widening_type_v<T, U>, int> = 0>
    explicit constexpr operator StrongNumber<U, Tag>() const noexcept {
        return StrongNumber<U, Tag>(static_cast<U>(value_));
    }

    // (C) EXPLICIT: to primitive integral
    template <typename U,
              typename = std::enable_if_t<std::is_integral_v<U>>>
    explicit constexpr operator U() const noexcept {
        return static_cast<U>(value_);
    }

    // (D) EXPLICIT: to primitive floating
    template <typename U,
              typename = std::enable_if_t<std::is_floating_point_v<U>>,
              typename Dummy = void>
    explicit constexpr operator U() const noexcept {
        return static_cast<U>(value_);
    }

    // ------------------------
    // ObjInterface methods
    // ------------------------

    std::string to_string() const {
        return std::to_string(value_);
    }

    std::string to_full_string() const {
        return "[StrongNumber value=" + std::to_string(value_) + "]";
    }

    // Text conversion methods - declared after including pyl_text.h

    constexpr std::size_t length() const noexcept {
        return 1;  // Scalar types have length 1
    }

    constexpr std::size_t hash() const noexcept {
        return std::hash<T>{}(value_);
    }

    // ------------------------
    // ObjTemplateInterface methods
    // ------------------------

    constexpr bool equals(const StrongNumber& other) const noexcept {
        return value_ == other.value_;
    }

    constexpr bool full_equals(const StrongNumber& other) const noexcept {
        return equals(other);
    }

    constexpr StrongNumber full_copy() const noexcept {
        return StrongNumber{value_};
    }

private:
    T value_{};
};

// common-type helper
template <typename T, typename U, typename Tag>
using StrongCommon = StrongNumber<std::common_type_t<T, U>, Tag>;

// arithmetic (same Tag, any T/U)
template <typename T, typename U, typename Tag>
constexpr StrongCommon<T, U, Tag>
operator+(const StrongNumber<T, Tag>& a, const StrongNumber<U, Tag>& b) {
    using R = std::common_type_t<T, U>;
    return StrongCommon<T, U, Tag>(
        static_cast<R>(a.value()) + static_cast<R>(b.value()));
}

template <typename T, typename U, typename Tag>
constexpr StrongCommon<T, U, Tag>
operator-(const StrongNumber<T, Tag>& a, const StrongNumber<U, Tag>& b) {
    using R = std::common_type_t<T, U>;
    return StrongCommon<T, U, Tag>(
        static_cast<R>(a.value()) - static_cast<R>(b.value()));
}

template <typename T, typename U, typename Tag>
constexpr StrongCommon<T, U, Tag>
operator*(const StrongNumber<T, Tag>& a, const StrongNumber<U, Tag>& b) {
    using R = std::common_type_t<T, U>;
    return StrongCommon<T, U, Tag>(
        static_cast<R>(a.value()) * static_cast<R>(b.value()));
}

template <typename T, typename U, typename Tag>
constexpr StrongCommon<T, U, Tag>
operator/(const StrongNumber<T, Tag>& a, const StrongNumber<U, Tag>& b) {
    using R = std::common_type_t<T, U>;
    return StrongCommon<T, U, Tag>(
        static_cast<R>(a.value()) / static_cast<R>(b.value()));
}

// comparisons (same Tag, any T/U)
template <typename T, typename U, typename Tag>
constexpr bool operator==(const StrongNumber<T, Tag>& a,
                          const StrongNumber<U, Tag>& b) noexcept {
    using R = std::common_type_t<T, U>;
    return static_cast<R>(a.value()) == static_cast<R>(b.value());
}

template <typename T, typename U, typename Tag>
constexpr bool operator!=(const StrongNumber<T, Tag>& a,
                          const StrongNumber<U, Tag>& b) noexcept {
    return !(a == b);
}

template <typename T, typename U, typename Tag>
constexpr bool operator<(const StrongNumber<T, Tag>& a,
                         const StrongNumber<U, Tag>& b) noexcept {
    using R = std::common_type_t<T, U>;
    return static_cast<R>(a.value()) < static_cast<R>(b.value());
}

template <typename T, typename U, typename Tag>
constexpr bool operator>(const StrongNumber<T, Tag>& a,
                         const StrongNumber<U, Tag>& b) noexcept {
    return b < a;
}

template <typename T, typename U, typename Tag>
constexpr bool operator<=(const StrongNumber<T, Tag>& a,
                          const StrongNumber<U, Tag>& b) noexcept {
    return !(b < a);
}

template <typename T, typename U, typename Tag>
constexpr bool operator>=(const StrongNumber<T, Tag>& a,
                          const StrongNumber<U, Tag>& b) noexcept {
    return !(a < b);
}

// ostream
template <typename T, typename Tag>
inline std::ostream& operator<<(std::ostream& os,
                                const StrongNumber<T, Tag>& v) {
    return os << v.value();
}

} // namespace pyl

// =========================
// 5. STRONG_NUM macro
//    STRONG_NUM(Name)
//    STRONG_NUM(Name, Kind)
//    STRONG_NUM(Name, Kind, Bits)
// =========================

#define STRONG_NUM_IMPL(BaseName, Kind, Bits)                                   \
    struct BaseName##Tag {};                                                    \
    using BaseName =                                                            \
        pyl::StrongNumber<pyl::NumTypeT<Kind, Bits>, BaseName##Tag>

// 1-arg: default Int, B32
#define STRONG_NUM_1(BaseName) \
    STRONG_NUM_IMPL(BaseName, pyl::NumKind::Int, pyl::NumBits::B32)

// 2-arg: user Kind, default B32
#define STRONG_NUM_2(BaseName, Kind) \
    STRONG_NUM_IMPL(BaseName, Kind, pyl::NumBits::B32)

// 3-arg: user Kind and Bits
#define STRONG_NUM_3(BaseName, Kind, Bits) \
    STRONG_NUM_IMPL(BaseName, Kind, Bits)

// pick correct helper by arg count
#define STRONG_NUM_GET_MACRO(_1,_2,_3,NAME,...) NAME
#define STRONG_NUM_DISPATCH(...) \
    STRONG_NUM_GET_MACRO(__VA_ARGS__, STRONG_NUM_3, STRONG_NUM_2, STRONG_NUM_1)

// public macro
#define STRONG_NUM(...) STRONG_NUM_DISPATCH(__VA_ARGS__)(__VA_ARGS__)

// Note: to_text() and to_full_text() for StrongNumber are provided by
// the generic implementations in pyl_text.h which call to_string()

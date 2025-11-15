#pragma once

#include <string>
#include <type_traits>
#include <utility>
#include <sstream>
#include <typeinfo>
#include <concepts>
#include <ostream>
#include <array>
#include <any>
#include <iostream>
#include <string_view>
#include <unordered_map>

namespace pyl {

// ---------------------------------------------------------
// Text – Python-like string concatenation with automatic type conversion
//
// Usage:
//   Text a = "Hello";
//   Text b = "World";
//   auto c = a + " " + b;           // Text
//   auto d = 1 + a + b;             // Text (auto-converts int to Text)
//   auto e = a + 42 + " items";     // Text
// ---------------------------------------------------------

class Text {
private:
    std::string data_;

public:
    // Constructors
    Text() = default;

    Text(const std::string& s) : data_(s) {}
    Text(std::string&& s) : data_(std::move(s)) {}
    Text(const char* s) : data_(s) {}

    // Access underlying string
    const std::string& str() const { return data_; }
    std::string& str() { return data_; }

    const char* c_str() const { return data_.c_str(); }

    // Size and empty checks
    size_t size() const { return data_.size(); }
    size_t length() const { return data_.size(); }  // Alias for ObjInterface
    bool empty() const { return data_.empty(); }

    // Implicit conversion to std::string for convenience
    operator const std::string&() const { return data_; }

    // Comparison operators
    bool operator==(const Text& other) const { return data_ == other.data_; }
    bool operator!=(const Text& other) const { return data_ != other.data_; }
    bool operator<(const Text& other) const { return data_ < other.data_; }
    bool operator<=(const Text& other) const { return data_ <= other.data_; }
    bool operator>(const Text& other) const { return data_ > other.data_; }
    bool operator>=(const Text& other) const { return data_ >= other.data_; }

    // ObjInterface methods
    std::string to_string() const { return data_; }
    std::string to_full_string() const {
        return "[Text value=\"" + data_ + "\"]";
    }

    Text to_text() const { return *this; }
    Text to_full_text() const { return Text{to_full_string()}; }

    std::size_t hash() const {
        return std::hash<std::string>{}(data_);
    }

    // ObjTemplateInterface methods
    bool equals(const Text& other) const {
        return data_ == other.data_;
    }

    bool full_equals(const Text& other) const {
        return equals(other);
    }

    Text full_copy() const {
        return Text{data_};
    }

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const Text& t) {
        return os << t.data_;
    }
};

// ---------------------------------------------------------
// to_text – Convert various types to Text
// ---------------------------------------------------------

// Arithmetic types (int, double, float, etc.)
template <typename T,
          typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>>>>
inline Text to_text(T value) {
    return Text{std::to_string(value)};
}

// String types
inline Text to_text(const std::string& s) {
    return Text{s};
}

inline Text to_text(std::string&& s) {
    return Text{std::move(s)};
}

inline Text to_text(const char* s) {
    return Text{s};
}

// Text itself (identity)
inline Text to_text(const Text& t) {
    return t;
}

inline Text to_text(Text&& t) {
    return std::move(t);
}

// ---------------------------------------------------------
// to_text – Generic conversion using multiple strategies (C++20)
//
// Priority order:
//   1. If T has to_string() method → use it
//   2. If T has operator<< → use stringstream
//   3. Fallback → type name + address
// ---------------------------------------------------------

// Generic to_text for any type T (uses C++20 requires)
template <typename T>
    requires (!std::is_arithmetic_v<std::decay_t<T>> &&
              !std::is_same_v<std::decay_t<T>, std::string> &&
              !std::is_same_v<std::decay_t<T>, Text> &&
              !std::is_same_v<std::decay_t<T>, const char*> &&
              !std::is_same_v<std::decay_t<T>, char*>)
inline Text to_text(const T& value) {
    // Strategy 1: T::to_string()
    if constexpr (requires(const T& v) {
        { v.to_string() } -> std::convertible_to<std::string>;
    }) {
        return Text{value.to_string()};
    }
    // Strategy 2: operator<<
    else if constexpr (requires(std::ostream& os, const T& v) {
        { os << v } -> std::same_as<std::ostream&>;
    }) {
        std::ostringstream oss;
        oss << value;
        return Text{oss.str()};
    }
    // Strategy 3: Fallback to type name + address
    else {
        std::ostringstream oss;
        oss << "<" << typeid(T).name()
            << " @" << static_cast<const void*>(&value) << ">";
        return Text{oss.str()};
    }
}

// Pointer overload
template <typename T>
inline Text to_text(T* ptr) {
    if (!ptr) {
        return Text{"<null>"};
    }

    // If pointee has to_string(), use it
    if constexpr (requires(const T& v) {
        { v.to_string() } -> std::convertible_to<std::string>;
    }) {
        return Text{ptr->to_string()};
    }
    // If pointee has operator<<
    else if constexpr (requires(std::ostream& os, const T& v) {
        { os << v } -> std::same_as<std::ostream&>;
    }) {
        std::ostringstream oss;
        oss << *ptr;
        return Text{oss.str()};
    }
    // Fallback: show pointer address
    else {
        std::ostringstream oss;
        oss << "<" << typeid(T).name() << " @" << static_cast<const void*>(ptr) << ">";
        return Text{oss.str()};
    }
}

// ---------------------------------------------------------
// to_text_full – Full conversion with type information
//
// Returns: [TypeName value=<string_representation>]
// Similar to to_full_string() in child_unique_ptr
// ---------------------------------------------------------

template <typename T>
    requires (!std::is_arithmetic_v<std::decay_t<T>> &&
              !std::is_same_v<std::decay_t<T>, std::string> &&
              !std::is_same_v<std::decay_t<T>, Text> &&
              !std::is_same_v<std::decay_t<T>, const char*> &&
              !std::is_same_v<std::decay_t<T>, char*>)
inline Text to_text_full(const T& value) {
    std::ostringstream oss;
    oss << "[" << typeid(T).name() << " value=";

    // Get string representation using to_text
    oss << to_text(value).str();

    oss << "]";
    return Text{oss.str()};
}

// Pointer overload for to_text_full
template <typename T>
inline Text to_text_full(T* ptr) {
    if (!ptr) {
        return Text{"[" + std::string(typeid(T*).name()) + " value=<null>]"};
    }

    std::ostringstream oss;
    oss << "[" << typeid(T).name() << "* ";
    oss << "value=" << to_text(*ptr).str();
    oss << " @" << static_cast<const void*>(ptr);
    oss << "]";
    return Text{oss.str()};
}

// Text specializations for to_text_full
inline Text to_text_full(const Text& t) {
    return Text{"[Text value=\"" + t.str() + "\"]"};
}

inline Text to_text_full(const std::string& s) {
    return Text{"[std::string value=\"" + s + "\"]"};
}

inline Text to_text_full(const char* s) {
    if (!s) {
        return Text{"[const char* value=<null>]"};
    }
    return Text{"[const char* value=\"" + std::string(s) + "\"]"};
}

// Arithmetic types for to_text_full
template <typename T>
    requires std::is_arithmetic_v<std::decay_t<T>>
inline Text to_text_full(T value) {
    std::ostringstream oss;
    oss << "[" << typeid(T).name() << " value=" << std::to_string(value) << "]";
    return Text{oss.str()};
}

// ---------------------------------------------------------
// to_text and to_text_full for child_unique_ptr
// Needs forward declaration of child_unique_ptr from pyl_child_ptr.h
// ---------------------------------------------------------

// Forward declaration
template<typename Parent, typename T, typename Deleter>
class child_unique_ptr;

// to_text for child_unique_ptr - uses the pointer's to_string() method
template <typename Parent, typename T, typename Deleter>
inline Text to_text(const child_unique_ptr<Parent, T, Deleter>& ptr) {
    return Text{ptr.to_string()};
}

// to_text_full for child_unique_ptr - uses the pointer's to_full_string() method
template <typename Parent, typename T, typename Deleter>
inline Text to_text_full(const child_unique_ptr<Parent, T, Deleter>& ptr) {
    return Text{ptr.to_full_string()};
}

// ---------------------------------------------------------
// operator+ – Text concatenation with automatic conversion
//
// These overloads allow:
//   - Text + Text
//   - Text + any_type  (converts any_type to Text)
//   - any_type + Text  (converts any_type to Text)
//
// Example:
//   Text a = "value: ";
//   auto b = a + 42;        // "value: 42"
//   auto c = 1 + a + 2;     // "1value: 2"
// ---------------------------------------------------------

// Text + Text
inline Text operator+(const Text& lhs, const Text& rhs) {
    return Text{lhs.str() + rhs.str()};
}

// Text + T (where T is not Text)
template <typename T,
          typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Text>>>
inline Text operator+(const Text& lhs, T&& rhs) {
    return lhs + to_text(std::forward<T>(rhs));
}

// T + Text (where T is not Text)
template <typename T,
          typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Text>>>
inline Text operator+(T&& lhs, const Text& rhs) {
    return to_text(std::forward<T>(lhs)) + rhs;
}

// =========================================================================
// Python-like formatted printing with automatic variable capture
//
// Usage:
//   int x = 42;
//   double y = 3.14;
//   F("x={x}, y={y}\n", x, y);
//
// No need for FIELD() macros at call site - variable names
// are automatically captured and matched to placeholders.
// =========================================================================

// ===================== Token & ParsedFormat =====================

enum class TokenKind { Text, Placeholder };

struct Token {
    TokenKind kind;
    std::string_view sv;  // points into the original format literal
};

template <std::size_t N>
struct ParsedFormat {
    std::array<Token, N> tokens{};
    std::size_t count = 0;
};

// ===================== Format parser =====================

template <std::size_t N>
inline ParsedFormat<N> parse_format(const char (&fmt)[N]) {
    ParsedFormat<N> pf{};
    std::size_t i   = 0;
    std::size_t tok = 0;

    // N includes trailing '\0', so valid chars are [0, N-2]
    while (i < N - 1) {
        if (fmt[i] == '{') {
            std::size_t j = i + 1;
            while (j < N - 1 && fmt[j] != '}')
                ++j;

            if (j >= N - 1) {
                // unmatched '{' → treat rest as text
                pf.tokens[tok++] = {
                    TokenKind::Text,
                    std::string_view(&fmt[i], (N - 1) - i)
                };
                break;
            }

            // placeholder token (without braces)
            pf.tokens[tok++] = {
                TokenKind::Placeholder,
                std::string_view(&fmt[i + 1], j - (i + 1))
            };
            i = j + 1;
        } else {
            // text chunk until next '{' or end
            std::size_t start = i;
            while (i < N - 1 && fmt[i] != '{')
                ++i;

            pf.tokens[tok++] = {
                TokenKind::Text,
                std::string_view(&fmt[start], i - start)
            };
        }
    }

    pf.count = tok;
    return pf;
}

// ===================== Runtime formatting =====================

using FieldMap = std::unordered_map<std::string, std::any>;

// Implemented in pyl_text.cpp
void add_field(FieldMap& m, std::string name, std::any value);
std::string any_to_string(const std::any& a);

template <class... Pairs>
FieldMap make_field_map(Pairs&&... pairs) {
    FieldMap m;
    (add_field(m,
               std::forward<Pairs>(pairs).first,
               std::forward<Pairs>(pairs).second), ...);
    return m;
}

template <std::size_t N>
std::string format_with_parsed(const ParsedFormat<N>& pf,
                               const FieldMap& fields) {
    std::string out;
    for (std::size_t i = 0; i < pf.count; ++i) {
        const Token& t = pf.tokens[i];
        if (t.kind == TokenKind::Text) {
            out.append(t.sv);
        } else { // Placeholder
            auto it = fields.find(std::string(t.sv));
            if (it != fields.end()) {
                out += any_to_string(it->second);
            } else {
                // keep unknown placeholder as-is
                out.push_back('{');
                out.append(t.sv);
                out.push_back('}');
            }
        }
    }
    return out;
}

// ===================== __F core =====================

template <std::size_t N>
void __F(const char (&fmt)[N], const FieldMap& fields) {
    // Parse format string
    auto parsed = parse_format(fmt);

    // Format using pre-parsed tokens
    auto result = format_with_parsed(parsed, fields);
    std::cerr << result;
}

} // namespace pyl

// ===================== Macro layer: args → ("name", value) =====================

// Token pasting & expansion helpers
#define CAT_INNER(a, b) a##b
#define CAT(a, b) CAT_INNER(a, b)
#define EXPAND(x) x

// Map one identifier to ("name", std::any(value))
#define MAKE_FIELD(x) std::make_pair(std::string(#x), std::any(x))

// ---- argument counting up to 8 ----
#define PP_RSEQ_N() 8,7,6,5,4,3,2,1,0
#define PP_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_NARG(...)  PP_NARG_(__VA_ARGS__, PP_RSEQ_N())

// ---- FOR_EACH to apply MAKE_FIELD to each arg ----
#define FOR_EACH_1(M, x1)            M(x1)
#define FOR_EACH_2(M, x1, ...)       M(x1), FOR_EACH_1(M, __VA_ARGS__)
#define FOR_EACH_3(M, x1, ...)       M(x1), FOR_EACH_2(M, __VA_ARGS__)
#define FOR_EACH_4(M, x1, ...)       M(x1), FOR_EACH_3(M, __VA_ARGS__)
#define FOR_EACH_5(M, x1, ...)       M(x1), FOR_EACH_4(M, __VA_ARGS__)
#define FOR_EACH_6(M, x1, ...)       M(x1), FOR_EACH_5(M, __VA_ARGS__)
#define FOR_EACH_7(M, x1, ...)       M(x1), FOR_EACH_6(M, __VA_ARGS__)
#define FOR_EACH_8(M, x1, ...)       M(x1), FOR_EACH_7(M, __VA_ARGS__)

#define FOR_EACH_DISPATCH(N, M, ...) EXPAND(CAT(FOR_EACH_, N)(M, __VA_ARGS__))
#define FOR_EACH(M, ...)             FOR_EACH_DISPATCH(PP_NARG(__VA_ARGS__), M, __VA_ARGS__)

// x, y, z → MAKE_FIELD(x), MAKE_FIELD(y), MAKE_FIELD(z)
#define MAKE_FIELD_ARGS(...) FOR_EACH(MAKE_FIELD, __VA_ARGS__)

// ===================== Public F macro =====================

// internal helper: build map and call __F
#define F_IMPL(line, fmt, ...)                                                   \
    auto CAT(__fmap_, line) = pyl::make_field_map(MAKE_FIELD_ARGS(__VA_ARGS__)); \
    pyl::__F(fmt, CAT(__fmap_, line))

// Public API:
//   F("x={x}, y={y}", x, y);
// (requires at least one variable after fmt)
#define F(fmt, ...) F_IMPL(__LINE__, fmt, __VA_ARGS__)

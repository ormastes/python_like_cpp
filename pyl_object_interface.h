#pragma once

#include <string>
#include <cstddef>
#include <concepts>

namespace pyl {

// Forward declaration of Text
class Text;

// ========================================
// ObjInterface Concept
// ========================================
// Defines the protocol that all pyl objects should follow
// Types implementing this concept should provide:
//
// Required methods:
//   - std::string to_string() const
//   - std::string to_full_string() const
//   - Text to_text() const
//   - Text to_full_text() const
//   - std::size_t length() const
//   - std::size_t hash() const
//
// This is a documentation interface - types don't need to inherit from it,
// they just need to provide these methods.

// ========================================
// ObjTemplateInterface Concept
// ========================================
// Defines type-specific operations for template types
//
// Required methods for type T:
//   - bool equals(const T& other) const
//   - bool full_equals(const T& other) const
//   - T full_copy() const
//
// This is a documentation interface - types don't need to inherit from it,
// they just need to provide these methods.

// ========================================
// Concept definitions for compile-time checking
// ========================================

template<typename T>
concept HasToString = requires(const T& t) {
    { t.to_string() } -> std::convertible_to<std::string>;
};

template<typename T>
concept HasToFullString = requires(const T& t) {
    { t.to_full_string() } -> std::convertible_to<std::string>;
};

template<typename T>
concept HasLength = requires(const T& t) {
    { t.length() } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept HasHash = requires(const T& t) {
    { t.hash() } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept HasEquals = requires(const T& a, const T& b) {
    { a.equals(b) } -> std::convertible_to<bool>;
};

template<typename T>
concept HasFullEquals = requires(const T& a, const T& b) {
    { a.full_equals(b) } -> std::convertible_to<bool>;
};

template<typename T>
concept HasFullCopy = requires(const T& t) {
    { t.full_copy() } -> std::convertible_to<T>;
};

// Complete object interface concept
template<typename T>
concept ObjInterfaceComplete =
    HasToString<T> &&
    HasToFullString<T> &&
    HasLength<T> &&
    HasHash<T>;

template<typename T>
concept ObjTemplateInterfaceComplete =
    HasEquals<T> &&
    HasFullEquals<T> &&
    HasFullCopy<T>;

} // namespace pyl

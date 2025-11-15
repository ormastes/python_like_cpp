#pragma once

#include <ranges>
#include <vector>
#include <utility>
#include <tuple>
#include <cstddef>
#include <functional>
#include <algorithm>
#include <type_traits>

namespace pyl {

// ---------------------------------------------------------
// Pair-like helpers: keys(range), values(range), pairs(range)
// Works for map, unordered_map, vector<pair<>>, list<pair<>>, ...
// ---------------------------------------------------------

template <class R>
concept PairLikeRange =
    std::ranges::input_range<R> &&
    requires (std::ranges::range_reference_t<R> ref) {
        std::get<0>(ref);
        std::get<1>(ref);
    };

// keys(range) -> view of first element of each pair
template <PairLikeRange R>
auto keys(R&& r) {
    return std::forward<R>(r)
         | std::views::transform([](auto&& kv) -> decltype(auto) {
               return std::get<0>(std::forward<decltype(kv)>(kv));
           });
}

// values(range) -> view of second element of each pair
template <PairLikeRange R>
auto values(R&& r) {
    return std::forward<R>(r)
         | std::views::transform([](auto&& kv) -> decltype(auto) {
               return std::get<1>(std::forward<decltype(kv)>(kv));
           });
}

// pairs(range) – identity, but keeps the symmetric API
template <PairLikeRange R>
auto pairs(R&& r) {
    return std::forward<R>(r);
}

// ---------------------------------------------------------
// to_vector(range): materialize any input_range into std::vector
// ---------------------------------------------------------

template <std::ranges::input_range R>
auto to_vector(R&& r) {
    using T = std::ranges::range_value_t<R>;
    std::vector<T> out;
    for (auto&& e : r) {
        out.emplace_back(e);
    }
    return out;
}

// ---------------------------------------------------------
// reduce / sum – Python-like reduction helpers
// ---------------------------------------------------------

// generic reduce(range, init, op)
template <std::ranges::input_range R, class T, class Op>
T reduce(R&& r, T init, Op op) {
    for (auto&& x : r) {
        init = op(std::move(init), x);
    }
    return init;
}

// reduce(range, init) – uses operator+
template <std::ranges::input_range R, class T>
T reduce(R&& r, T init) {
    return reduce(std::forward<R>(r), std::move(init), std::plus<>{});
}

// sum(range) – assumes default-constructible value type and +
template <std::ranges::input_range R>
auto sum(R&& r) {
    using V = std::ranges::range_value_t<R>;
    return reduce(std::forward<R>(r), V{});
}

// ---------------------------------------------------------
// ANY / ALL – Python-like any() / all()
// ---------------------------------------------------------

#define ANY(range, var, expr) \
    std::ranges::any_of((range), [&](const auto& var) { return (expr); })

#define ALL(range, var, expr) \
    std::ranges::all_of((range), [&](const auto& var) { return (expr); })

// ---------------------------------------------------------
// IF – Python-style filtering
//
//   IF(x,   expr)     -> filter on a single value
//   IF(k,v, expr)     -> filter on pair-like (k, v)
// ---------------------------------------------------------

// 1-var version: IF(x, expr)
#define IF2(var, expr) \
    std::views::filter([&](const auto& var) { return (expr); })

// 2-var version: IF(k, v, expr)
#define IF3(k, v, expr) \
    std::views::filter([&](const auto& __kv) { \
        const auto& k = std::get<0>(__kv);     \
        const auto& v = std::get<1>(__kv);     \
        return (expr);                         \
    })

#define GET_MACRO_IF(_1, _2, _3, NAME, ...) NAME
#define IF(...) GET_MACRO_IF(__VA_ARGS__, IF3, IF2)(__VA_ARGS__)

// ---------------------------------------------------------
// MAP – Python-style transform
//
//   MAP(x,   expr)    -> transform value
//   MAP(k,v, expr)    -> transform from pair-like (k, v)
// ---------------------------------------------------------

// 1-var version: MAP(x, expr)
#define MAP2(var, expr) \
    std::views::transform([&](auto&& var) { return (expr); })

// 2-var version: MAP(k, v, expr)
#define MAP3(k, v, expr) \
    std::views::transform([&](auto&& __kv) { \
        auto&& k = std::get<0>(__kv);        \
        auto&& v = std::get<1>(__kv);        \
        return (expr);                       \
    })

#define GET_MACRO_MAP(_1, _2, _3, NAME, ...) NAME
#define MAP(...) GET_MACRO_MAP(__VA_ARGS__, MAP3, MAP2)(__VA_ARGS__)

// ---------------------------------------------------------
// ENUM – Python-style enumerate()
//
//   range | ENUM(i, x)       -> (index, value)
//   range | ENUM(i, k, v)    -> (index, key, value) for pair-like
// ---------------------------------------------------------

// ENUM(i, x)
#define ENUM2(i, x) \
    std::views::transform([i = std::size_t{0}](auto&& x) mutable { \
        return std::pair(i++, std::forward<decltype(x)>(x));       \
    })

// ENUM(i, k, v) – for pair-like
#define ENUM3(i, k, v) \
    std::views::transform([i = std::size_t{0}](auto&& __kv) mutable { \
        auto&& k = std::get<0>(__kv);                                 \
        auto&& v = std::get<1>(__kv);                                 \
        return std::tuple(i++, k, v);                                 \
    })

#define GET_MACRO_ENUM(_1, _2, _3, NAME, ...) NAME
#define ENUM(...) GET_MACRO_ENUM(__VA_ARGS__, ENUM3, ENUM2)(__VA_ARGS__)

} // namespace pyl

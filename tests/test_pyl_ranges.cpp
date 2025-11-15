#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <map>
#include <string>
#include "pyl_ranges.h"

using namespace pyl;

TEST_CASE("to_vector materializes ranges", "[pyl_ranges]") {
    std::vector<int> input{1, 2, 3, 4, 5};

    auto doubled = to_vector(
        input | std::views::transform([](int x) { return x * 2; })
    );

    REQUIRE(doubled.size() == 5);
    REQUIRE(doubled[0] == 2);
    REQUIRE(doubled[4] == 10);
}

TEST_CASE("IF macro filters ranges", "[pyl_ranges]") {
    std::vector<int> nums{1, 2, 3, 4, 5, 6};

    auto evens = to_vector(nums | IF(x, x % 2 == 0));

    REQUIRE(evens.size() == 3);
    REQUIRE(evens[0] == 2);
    REQUIRE(evens[1] == 4);
    REQUIRE(evens[2] == 6);
}

TEST_CASE("MAP macro transforms ranges", "[pyl_ranges]") {
    std::vector<int> nums{1, 2, 3};

    auto squared = to_vector(nums | MAP(x, x * x));

    REQUIRE(squared.size() == 3);
    REQUIRE(squared[0] == 1);
    REQUIRE(squared[1] == 4);
    REQUIRE(squared[2] == 9);
}

TEST_CASE("IF and MAP can be combined", "[pyl_ranges]") {
    std::vector<int> nums{1, 2, 3, 4, 5, 6};

    // Get squares of even numbers
    auto result = to_vector(
        nums
        | IF(x, x % 2 == 0)
        | MAP(x, x * x)
    );

    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 4);   // 2^2
    REQUIRE(result[1] == 16);  // 4^2
    REQUIRE(result[2] == 36);  // 6^2
}

TEST_CASE("sum reduces ranges", "[pyl_ranges]") {
    std::vector<int> nums{1, 2, 3, 4, 5};

    int total = sum(nums);

    REQUIRE(total == 15);
}

TEST_CASE("reduce with custom operation", "[pyl_ranges]") {
    std::vector<int> nums{1, 2, 3, 4, 5};

    int product = reduce(nums, 1, [](int acc, int x) { return acc * x; });

    REQUIRE(product == 120);
}

TEST_CASE("ANY macro checks if any element matches", "[pyl_ranges]") {
    std::vector<int> nums{1, 3, 5, 7, 9};

    bool has_even = ANY(nums, x, x % 2 == 0);
    bool has_positive = ANY(nums, x, x > 0);

    REQUIRE_FALSE(has_even);
    REQUIRE(has_positive);
}

TEST_CASE("ALL macro checks if all elements match", "[pyl_ranges]") {
    std::vector<int> nums{2, 4, 6, 8};

    bool all_even = ALL(nums, x, x % 2 == 0);
    bool all_positive = ALL(nums, x, x > 0);
    bool all_negative = ALL(nums, x, x < 0);

    REQUIRE(all_even);
    REQUIRE(all_positive);
    REQUIRE_FALSE(all_negative);
}

TEST_CASE("keys extracts keys from map", "[pyl_ranges]") {
    std::map<std::string, int> m{
        {"a", 1},
        {"b", 2},
        {"c", 3}
    };

    auto key_vec = to_vector(keys(m));

    REQUIRE(key_vec.size() == 3);
    REQUIRE(key_vec[0] == "a");
    REQUIRE(key_vec[1] == "b");
    REQUIRE(key_vec[2] == "c");
}

TEST_CASE("values extracts values from map", "[pyl_ranges]") {
    std::map<std::string, int> m{
        {"a", 1},
        {"b", 2},
        {"c", 3}
    };

    auto val_vec = to_vector(values(m));

    REQUIRE(val_vec.size() == 3);
    REQUIRE(val_vec[0] == 1);
    REQUIRE(val_vec[1] == 2);
    REQUIRE(val_vec[2] == 3);
}

TEST_CASE("pairs works with map", "[pyl_ranges]") {
    std::map<std::string, int> m{
        {"x", 10},
        {"y", 20}
    };

    int count = 0;
    for (auto&& [k, v] : pairs(m)) {
        count++;
        REQUIRE(m[k] == v);
    }

    REQUIRE(count == 2);
}

TEST_CASE("IF with key-value pairs", "[pyl_ranges]") {
    std::map<std::string, int> m{
        {"one", 1},
        {"two", 2},
        {"three", 3},
        {"four", 4}
    };

    auto even_keys = to_vector(
        m
        | IF(k, v, v % 2 == 0)
        | MAP(k, v, k)
    );

    REQUIRE(even_keys.size() == 2);
    REQUIRE(even_keys[0] == "four");
    REQUIRE(even_keys[1] == "two");
}

TEST_CASE("ENUM enumerates vector elements", "[pyl_ranges]") {
    std::vector<std::string> words{"hello", "world"};

    std::vector<std::pair<std::size_t, std::string>> result;
    for (auto [i, word] : words | ENUM(i, word)) {
        result.emplace_back(i, word);
    }

    REQUIRE(result.size() == 2);
    REQUIRE(result[0].first == 0);
    REQUIRE(result[0].second == "hello");
    REQUIRE(result[1].first == 1);
    REQUIRE(result[1].second == "world");
}

TEST_CASE("ENUM with map (3-arg version)", "[pyl_ranges]") {
    std::map<std::string, int> m{
        {"a", 10},
        {"b", 20}
    };

    std::vector<std::tuple<std::size_t, std::string, int>> result;
    for (auto [i, k, v] : m | ENUM(i, k, v)) {
        result.emplace_back(i, k, v);
    }

    REQUIRE(result.size() == 2);
    REQUIRE(std::get<0>(result[0]) == 0);
    REQUIRE(std::get<1>(result[0]) == "a");
    REQUIRE(std::get<2>(result[0]) == 10);
}

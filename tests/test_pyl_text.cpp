#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "pyl_text.h"

using namespace pyl;

// Custom test types
struct Point {
    int x, y;
    std::string to_string() const {
        return "Point(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

struct Color {
    int r, g, b;
    friend std::ostream& operator<<(std::ostream& os, const Color& c) {
        return os << "RGB(" << c.r << "," << c.g << "," << c.b << ")";
    }
};

TEST_CASE("Text basic construction", "[pyl_text]") {
    Text a;
    Text b = "Hello";
    Text c = std::string("World");

    REQUIRE(a.empty());
    REQUIRE(b.str() == "Hello");
    REQUIRE(c.str() == "World");
}

TEST_CASE("Text concatenation", "[pyl_text]") {
    Text a = "Hello";
    Text b = "World";

    auto c = a + " " + b;

    REQUIRE(c.str() == "Hello World");
}

TEST_CASE("Text with integer concatenation", "[pyl_text]") {
    Text a = "Count: ";
    auto b = a + 42;

    REQUIRE(b.str() == "Count: 42");
}

TEST_CASE("Integer with Text concatenation", "[pyl_text]") {
    Text a = "Hello";
    auto result = 1 + a;

    REQUIRE(result.str() == "1Hello");
}

TEST_CASE("Chained mixed type concatenation", "[pyl_text]") {
    Text a = "A";
    Text b = "B";
    auto result = 1 + a + b;

    REQUIRE(result.str() == "1AB");
}

TEST_CASE("Text comparison operators", "[pyl_text]") {
    Text a = "abc";
    Text b = "abc";
    Text c = "xyz";

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
    REQUIRE(a != c);
    REQUIRE(a < c);
    REQUIRE(a <= b);
    REQUIRE(c > a);
    REQUIRE(c >= a);
}

TEST_CASE("to_text with arithmetic types", "[pyl_text]") {
    REQUIRE(to_text(42).str() == "42");
    REQUIRE(to_text(3.14).str().substr(0, 4) == "3.14");
    REQUIRE(to_text(true).str() == "1");
}

TEST_CASE("to_text with strings", "[pyl_text]") {
    std::string s = "C++ String";
    const char* cs = "C String";

    REQUIRE(to_text(s).str() == "C++ String");
    REQUIRE(to_text(cs).str() == "C String");
}

TEST_CASE("to_text with custom type having to_string", "[pyl_text]") {
    Point p{10, 20};

    REQUIRE(to_text(p).str() == "Point(10, 20)");
}

TEST_CASE("to_text with custom type having operator<<", "[pyl_text]") {
    Color c{255, 128, 0};

    REQUIRE(to_text(c).str() == "RGB(255,128,0)");
}

TEST_CASE("to_text with nullptr", "[pyl_text]") {
    int* ptr = nullptr;

    REQUIRE(to_text(ptr).str() == "<null>");
}

TEST_CASE("to_text with valid pointer", "[pyl_text]") {
    Point p{5, 10};
    Point* ptr = &p;

    REQUIRE(to_text(ptr).str() == "Point(5, 10)");
}

TEST_CASE("to_text_full with arithmetic types", "[pyl_text]") {
    auto result = to_text_full(42).str();

    REQUIRE(result.find("value=42") != std::string::npos);
}

TEST_CASE("to_text_full with string", "[pyl_text]") {
    auto result = to_text_full(std::string("test")).str();

    REQUIRE(result.find("value=\"test\"") != std::string::npos);
}

TEST_CASE("to_text_full with Text", "[pyl_text]") {
    Text t = "sample";
    auto result = to_text_full(t).str();

    REQUIRE(result.find("Text") != std::string::npos);
    REQUIRE(result.find("value=\"sample\"") != std::string::npos);
}

TEST_CASE("to_text_full with custom type", "[pyl_text]") {
    Point p{1, 2};
    auto result = to_text_full(p).str();

    REQUIRE(result.find("Point(1, 2)") != std::string::npos);
}

TEST_CASE("to_text_full with pointer", "[pyl_text]") {
    Point p{7, 8};
    Point* ptr = &p;
    auto result = to_text_full(ptr).str();

    REQUIRE(result.find("Point(7, 8)") != std::string::npos);
    REQUIRE(result.find("@") != std::string::npos);  // Has address
}

TEST_CASE("Text size and empty", "[pyl_text]") {
    Text empty;
    Text hello = "Hello";

    REQUIRE(empty.size() == 0);
    REQUIRE(empty.empty());
    REQUIRE(hello.size() == 5);
    REQUIRE_FALSE(hello.empty());
}

TEST_CASE("Text c_str", "[pyl_text]") {
    Text t = "Test";

    REQUIRE(std::string(t.c_str()) == "Test");
}

TEST_CASE("Text stream output", "[pyl_text]") {
    Text t = "StreamTest";
    std::ostringstream oss;

    oss << t;

    REQUIRE(oss.str() == "StreamTest");
}

TEST_CASE("any_to_string with various types", "[pyl_text]") {
    std::any a_int = 42;
    std::any a_double = 3.14;
    std::any a_string = std::string("hello");
    std::any a_bool_true = true;
    std::any a_bool_false = false;
    std::any a_null;

    REQUIRE(any_to_string(a_int) == "42");
    REQUIRE(any_to_string(a_double).substr(0, 4) == "3.14");
    REQUIRE(any_to_string(a_string) == "hello");
    REQUIRE(any_to_string(a_bool_true) == "true");
    REQUIRE(any_to_string(a_bool_false) == "false");
    REQUIRE(any_to_string(a_null) == "<null>");
}

TEST_CASE("FieldMap operations", "[pyl_text]") {
    FieldMap m;
    add_field(m, "x", std::any(42));
    add_field(m, "name", std::any(std::string("Alice")));

    REQUIRE(m.size() == 2);
    REQUIRE(std::any_cast<int>(m["x"]) == 42);
    REQUIRE(std::any_cast<std::string>(m["name"]) == "Alice");
}

TEST_CASE("make_field_map from pairs", "[pyl_text]") {
    auto m = make_field_map(
        std::make_pair("a", std::any(1)),
        std::make_pair("b", std::any(2))
    );

    REQUIRE(m.size() == 2);
    REQUIRE(std::any_cast<int>(m["a"]) == 1);
    REQUIRE(std::any_cast<int>(m["b"]) == 2);
}

TEST_CASE("parse_format with simple text", "[pyl_text]") {
    const char fmt[] = "Hello World";
    auto parsed = parse_format(fmt);

    REQUIRE(parsed.count == 1);
    REQUIRE(parsed.tokens[0].kind == TokenKind::Text);
    REQUIRE(parsed.tokens[0].sv == "Hello World");
}

TEST_CASE("parse_format with placeholder", "[pyl_text]") {
    const char fmt[] = "x={x}";
    auto parsed = parse_format(fmt);

    REQUIRE(parsed.count == 2);
    REQUIRE(parsed.tokens[0].kind == TokenKind::Text);
    REQUIRE(parsed.tokens[0].sv == "x=");
    REQUIRE(parsed.tokens[1].kind == TokenKind::Placeholder);
    REQUIRE(parsed.tokens[1].sv == "x");
}

TEST_CASE("parse_format with multiple placeholders", "[pyl_text]") {
    const char fmt[] = "{x} and {y}";
    auto parsed = parse_format(fmt);

    REQUIRE(parsed.count == 3);
    REQUIRE(parsed.tokens[0].kind == TokenKind::Placeholder);
    REQUIRE(parsed.tokens[0].sv == "x");
    REQUIRE(parsed.tokens[1].kind == TokenKind::Text);
    REQUIRE(parsed.tokens[1].sv == " and ");
    REQUIRE(parsed.tokens[2].kind == TokenKind::Placeholder);
    REQUIRE(parsed.tokens[2].sv == "y");
}

TEST_CASE("format_with_parsed replaces placeholders", "[pyl_text]") {
    const char fmt[] = "x={x}, y={y}";
    auto parsed = parse_format(fmt);

    FieldMap fields;
    add_field(fields, "x", std::any(10));
    add_field(fields, "y", std::any(20));

    auto result = format_with_parsed(parsed, fields);

    REQUIRE(result == "x=10, y=20");
}

TEST_CASE("format_with_parsed handles unknown placeholders", "[pyl_text]") {
    const char fmt[] = "known={x}, unknown={z}";
    auto parsed = parse_format(fmt);

    FieldMap fields;
    add_field(fields, "x", std::any(42));

    auto result = format_with_parsed(parsed, fields);

    REQUIRE(result == "known=42, unknown={z}");
}

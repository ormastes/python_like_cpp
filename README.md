# Python-like C++ Library (pyl)

A modern C++20 library that brings Python-like syntax and convenience to C++, providing intuitive range operations, text handling, smart pointers with parent tracking, and type-safe numeric types.

## Features

- **Python-style range operations** with macros for filtering, mapping, and more
- **Type-safe Text class** for string manipulation
- **Parent-aware smart pointers** (`child_unique_ptr`) with cycle detection
- **Strong numeric types** with automatic widening conversions
- **Rust-like type aliases** (u8, u16, i32, i64, f32, f64, etc.)
- **Unified object interface** using C++20 concepts
- **Header-only** components (except Text which has a .cpp implementation)

## Components

### pyl_ranges.h

Python-style range operations using C++20 ranges with convenient macros:

```cpp
#include "pyl_ranges.h"

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Filter even numbers and double them
auto result = numbers
    | IF(x, x % 2 == 0)      // Keep only even numbers
    | MAP(x, x * 2)           // Double each value
    | to_vector();            // Materialize to vector

// result = {4, 8, 12, 16, 20}

// Work with maps
std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 80}, {"Charlie", 92}};

auto high_scorers = scores
    | IF(k, v, v >= 90)       // Filter by value
    | MAP(k, v, k)            // Extract keys
    | to_vector();

// Aggregations
int sum = numbers | pyl::sum();
bool has_even = numbers | ANY(x, x % 2 == 0);
bool all_positive = numbers | ALL(x, x > 0);
```

### pyl_text.h

A convenient text class with Python-like string operations:

```cpp
#include "pyl_text.h"

using pyl::Text;

// Construction and concatenation
Text greeting = Text("Hello") + " " + "World";
Text message = Text("Count: ") + 42;

// Conversions
Text from_int = pyl::to_text(42);
Text from_ptr = pyl::to_text(&some_object);

// Comparison
if (text1 == text2) { /* ... */ }

// Access
const char* c_str = greeting.c_str();
size_t len = greeting.length();
```

### pyl_child_ptr.h

Smart pointers with parent tracking and cycle detection:

```cpp
#include "pyl_child_ptr.h"

struct Node {
    int value;
    pyl::child_unique_ptr<Node> left;
    pyl::child_unique_ptr<Node> right;

    Node(int v) : value(v), left(this), right(this) {}
};

Node root(1);
root.left.emplace(2);    // Automatically tracks parent
root.right.emplace(3);

// Cycle prevention - this is silently refused
root.left->left.reset(&root);  // Would create cycle, so it's prevented

// Dynamic fields (Python-like attributes)
root.left["custom_field"] = std::string("value");
auto field = std::any_cast<std::string>(root.left["custom_field"]);

// Query parent
if (root.left.has_parent()) {
    Node* parent = root.left.parent();
}
```

### pyl_strong_num.h

Type-safe numeric wrapper with configurable kind and bit width:

```cpp
#include "pyl_strong_num.h"

// Define custom numeric types
STRONG_NUM(UserId, pyl::NumKind::Uint, pyl::NumBits::B64);
STRONG_NUM(Temperature, pyl::NumKind::Float, pyl::NumBits::B32);
STRONG_NUM(Score);  // Defaults to signed 32-bit int

UserId id1(42);
UserId id2(100);
auto sum = id1 + id2;  // Type-safe: returns UserId

// Temperature temp = id1;  // Compile error: different types!

// Implicit widening allowed
pyl::StrongNumber<int32_t, SomeTag> small(42);
pyl::StrongNumber<int64_t, SomeTag> large = small;  // OK

// Explicit narrowing required
pyl::StrongNumber<int64_t, SomeTag> big(1000000);
auto small2 = static_cast<pyl::StrongNumber<int32_t, SomeTag>>(big);
```

### pyl_basic_types.h

Rust-like type aliases and user-defined literals:

```cpp
#include "pyl_basic_types.h"

using namespace pyl;

// Type aliases
u8  byte = 255;
u32 count = 1000;
i64 offset = -500;
f32 temperature = 98.6f;
f64 precision = 3.14159265359;

// User-defined literals
auto a = 42_u8;        // u8
auto b = 1000_u16;     // u16
auto c = 100_i32;      // i32
auto d = -50_i64;      // i64
auto e = 3.14_f32;     // f32
auto f = 2.71828_f64;  // f64

// Platform-dependent sizes
usize index = 0;       // size_t
isize diff = -10;      // ptrdiff_t
```

### pyl_object_interface.h

Unified object interface using C++20 concepts:

```cpp
#include "pyl_object_interface.h"

// All pyl types implement these concepts
template<typename T>
concept ObjInterfaceComplete =
    pyl::HasToString<T> &&
    pyl::HasToFullString<T> &&
    pyl::HasLength<T> &&
    pyl::HasHash<T>;

template<typename T>
concept ObjTemplateInterfaceComplete =
    pyl::HasEquals<T> &&
    pyl::HasFullEquals<T> &&
    pyl::HasFullCopy<T>;

// Works with any pyl type
template<pyl::ObjInterfaceComplete T>
void print_info(const T& obj) {
    std::cout << obj.to_string() << "\n";
    std::cout << "Length: " << obj.length() << "\n";
    std::cout << "Hash: " << obj.hash() << "\n";
}
```

## Building

### Requirements

- C++20 compatible compiler (Clang 15+, GCC 10+, MSVC 2019+)
- CMake 3.20 or higher
- Catch2 (automatically fetched by CMake)

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd python_like_cpp

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . -j4

# Run tests
ctest --output-on-failure
```

### Build with Coverage

```bash
# Configure with coverage enabled
cmake -DCMAKE_CXX_COMPILER=clang++ -DENABLE_COVERAGE=ON ..

# Build
cmake --build . -j4

# Generate coverage report
cmake --build . --target coverage

# View coverage report
# Open build/coverage_html/index.html in a browser
```

## Testing

The library includes comprehensive tests with 151 test cases covering:
- Range operations (filtering, mapping, reducing, etc.)
- Text construction, concatenation, and conversion
- Child pointer parent tracking and cycle detection
- Strong number arithmetic and type safety
- Basic type literals and conversions

### Coverage Results

- **pyl_basic_types.h**: 100% coverage
- **pyl_ranges.h**: 100% coverage
- **pyl_strong_num.h**: 100% coverage
- **pyl_child_ptr.h**: 87.50% regions, 100% functions
- **pyl_text**: 79-94% coverage across metrics

Run tests:
```bash
cd build
ctest --output-on-failure
```

Run specific test:
```bash
./pyl_tests "[test name]"
```

## Installation

```bash
cd build
cmake --install . --prefix /path/to/install
```

Installed files:
- Headers: `include/pyl_*.h`
- Library: `lib/libpyl.a` (or `.so`/`.dll`)

## Usage in Your Project

### With CMake

```cmake
find_package(pyl REQUIRED)
target_link_libraries(your_target PRIVATE pyl)
```

### Manual

```cpp
#include <pyl_ranges.h>
#include <pyl_text.h>
#include <pyl_child_ptr.h>
#include <pyl_strong_num.h>
#include <pyl_basic_types.h>
#include <pyl_object_interface.h>

// Link against libpyl for Text implementation
// Other components are header-only
```

## Design Principles

1. **Python-like convenience**: Intuitive syntax inspired by Python
2. **Type safety**: Strong typing with compile-time checks
3. **Zero-cost abstractions**: Header-only templates with no runtime overhead
4. **Modern C++**: Leverages C++20 features (concepts, ranges, etc.)
5. **Interoperability**: Works seamlessly with standard C++ types

## Examples

### Complete Example

```cpp
#include <pyl_ranges.h>
#include <pyl_text.h>
#include <pyl_basic_types.h>
#include <iostream>

using namespace pyl;

int main() {
    // Use basic types
    std::vector<i32> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Filter and transform
    auto evens = numbers
        | IF(x, x % 2 == 0)
        | MAP(x, x * x)
        | to_vector();

    // Create text
    Text message = Text("Even squares: ");
    for (auto n : evens) {
        message = message + to_text(n) + " ";
    }

    std::cout << message.c_str() << "\n";
    // Output: Even squares: 4 16 36 64 100

    return 0;
}
```

## License

[Specify your license here]

## Contributing

[Specify contribution guidelines here]

## Author

[Your name/organization]

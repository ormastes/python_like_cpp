#include <catch2/catch_test_macros.hpp>
#include "pyl_child_ptr.h"
#include "pyl_text.h"

using namespace pyl;

// Test Node structure
struct Node : Backtraceable<Node> {
    using child_ptr = child_unique_ptr<Node, Node>;

    int value = 0;
    child_ptr left{this};
    child_ptr right{this};

    explicit Node(int v) : value(v) {}

    std::string to_string() const {
        return "Node(" + std::to_string(value) + ")";
    }

    bool operator==(const Node& other) const {
        return value == other.value;
    }
};

TEST_CASE("child_unique_ptr basic construction", "[pyl_child_ptr]") {
    Node root(1);

    REQUIRE(root.value == 1);
    REQUIRE_FALSE(root.left);
    REQUIRE_FALSE(root.right);
}

TEST_CASE("child_unique_ptr emplace", "[pyl_child_ptr]") {
    Node root(1);

    root.left.emplace(2);
    root.right.emplace(3);

    REQUIRE(root.left);
    REQUIRE(root.right);
    REQUIRE(root.left->value == 2);
    REQUIRE(root.right->value == 3);
}

TEST_CASE("child_unique_ptr parent tracking", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    REQUIRE(root.left->parent == &root);
    REQUIRE(root.left->has_parent());
}

TEST_CASE("child_unique_ptr to_string", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    REQUIRE(root.left.to_string() == "Node(2)");
}

TEST_CASE("child_unique_ptr to_full_string includes parent info", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    std::string full = root.left.to_full_string();

    REQUIRE(full.find("Node(2)") != std::string::npos);
    REQUIRE(full.find("parent@") != std::string::npos);
}

TEST_CASE("to_text with child_unique_ptr", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    Text t = to_text(root.left);

    REQUIRE(t.str() == "Node(2)");
}

TEST_CASE("to_text_full with child_unique_ptr", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    Text t = to_text_full(root.left);

    REQUIRE(t.str().find("Node(2)") != std::string::npos);
    REQUIRE(t.str().find("parent@") != std::string::npos);
}

TEST_CASE("Text concatenation with child_unique_ptr", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);
    root.right.emplace(3);

    auto description = Text("Left: ") + to_text(root.left) +
                      ", Right: " + to_text(root.right);

    REQUIRE(description.str() == "Left: Node(2), Right: Node(3)");
}

TEST_CASE("child_unique_ptr dynamic fields - assignment", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    root.left["hp"] = 100;
    root.left["name"] = std::string("slime");
    root.left["alive"] = true;

    REQUIRE(root.left["hp"].exists());
    REQUIRE(root.left["name"].exists());
    REQUIRE(root.left["alive"].exists());
}

TEST_CASE("child_unique_ptr dynamic fields - retrieval", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    root.left["hp"] = 100;
    root.left["name"] = std::string("slime");
    root.left["alive"] = true;

    int hp = root.left["hp"];
    std::string name = root.left["name"];
    bool alive = root.left["alive"].as<bool>();

    REQUIRE(hp == 100);
    REQUIRE(name == "slime");
    REQUIRE(alive == true);
}

TEST_CASE("child_unique_ptr dynamic fields - exists check", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    REQUIRE_FALSE(root.left["hp"].exists());

    root.left["hp"] = 100;

    REQUIRE(root.left["hp"].exists());
    REQUIRE_FALSE(root.left["nonexistent"].exists());
}

TEST_CASE("child_unique_ptr dynamic functions - definition", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    root.left.def<int, int, int>("add", [](int a, int b) {
        return a + b;
    });

    int result = root.left.call<int>("add", 10, 20);

    REQUIRE(result == 30);
}

TEST_CASE("child_unique_ptr dynamic functions - field syntax", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    root.left.def<int, int, int>("multiply", [](int a, int b) {
        return a * b;
    });

    int result = root.left["multiply"](5, 6).as<int>();

    REQUIRE(result == 30);
}

TEST_CASE("child_unique_ptr hash", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);
    root.right.emplace(3);

    std::size_t hash1 = root.left.hash();
    std::size_t hash2 = root.right.hash();

    // Different nodes should (likely) have different hashes
    REQUIRE(hash1 != hash2);
}

TEST_CASE("child_unique_ptr equals", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);
    root.right.emplace(2);  // Same value as left

    REQUIRE(root.left.equals(root.right));
    REQUIRE(root.left.equals(root.left));
}

TEST_CASE("child_unique_ptr not equals", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);
    root.right.emplace(3);

    REQUIRE_FALSE(root.left.equals(root.right));
}

TEST_CASE("child_unique_ptr cycle prevention", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    // Try to create a cycle: root.left.left = &root
    root.left->left.reset(&root);

    // Cycle should be prevented
    REQUIRE_FALSE(root.left->left);
}

TEST_CASE("child_unique_ptr null pointer to_string", "[pyl_child_ptr]") {
    Node root(1);

    REQUIRE(root.left.to_string() == "<null>");
}

TEST_CASE("child_unique_ptr get and release", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    Node* ptr = root.left.get();
    REQUIRE(ptr != nullptr);
    REQUIRE(ptr->value == 2);

    Node* released = root.left.release();
    REQUIRE(released != nullptr);
    REQUIRE_FALSE(root.left);

    delete released;  // Manual cleanup after release
}

TEST_CASE("child_unique_ptr reset", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);

    REQUIRE(root.left->value == 2);

    root.left.reset(new Node(10));

    REQUIRE(root.left->value == 10);
}

TEST_CASE("child_unique_ptr swap", "[pyl_child_ptr]") {
    Node root(1);
    root.left.emplace(2);
    root.right.emplace(3);

    root.left.swap(root.right);

    REQUIRE(root.left->value == 3);
    REQUIRE(root.right->value == 2);
}

TEST_CASE("child_unique_ptr bool conversion", "[pyl_child_ptr]") {
    Node root(1);

    REQUIRE_FALSE(static_cast<bool>(root.left));

    root.left.emplace(2);

    REQUIRE(static_cast<bool>(root.left));
}

TEST_CASE("make_child_unique_ptr orphan", "[pyl_child_ptr]") {
    auto ptr = make_child_unique_ptr<Node>(5);

    REQUIRE(ptr);
    REQUIRE(ptr->value == 5);
    REQUIRE_FALSE(ptr->has_parent());
}

TEST_CASE("make_child_unique_ptr with parent", "[pyl_child_ptr]") {
    Node root(1);
    auto ptr = make_child_unique_ptr<Node>(&root, 7);

    REQUIRE(ptr);
    REQUIRE(ptr->value == 7);
    REQUIRE(ptr.parent() == &root);
    REQUIRE(ptr->parent == &root);  // Also check the back-pointer directly
}

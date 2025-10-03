#include <catch2/catch_test_macros.hpp>
#include "Vector.h"
#include <string>
#include <vector>

TEST_CASE("Vector construction", "[Vector]") {
    SECTION("Default construction") {
        const Vector<int> vec;
        REQUIRE(vec.size() == 0);
    }

    SECTION("Construction with different types") {
        const Vector<std::string> str_vec;
        REQUIRE(str_vec.size() == 0);

        const Vector<double> double_vec;
        REQUIRE(double_vec.size() == 0);
    }
}

TEST_CASE("Vector push_back operations", "[Vector]") {
    Vector<int> vec;

    SECTION("Push single element") {
        vec.push_back(42);
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == 42);
    }

    SECTION("Push multiple elements") {
        for (int i = 0; i < 10; ++i) {
            vec.push_back(i);
        }
        REQUIRE(vec.size() == 10);
        for (int i = 0; i < 10; ++i) {
            REQUIRE(vec[i] == i);
        }
    }

    SECTION("Push beyond initial capacity") {
        // Initial capacity is 8, so push 20 elements to trigger resize
        for (int i = 0; i < 20; ++i) {
            vec.push_back(i * 2);
        }
        REQUIRE(vec.size() == 20);
        for (int i = 0; i < 20; ++i) {
            REQUIRE(vec[i] == i * 2);
        }
    }
}

TEST_CASE("Vector emplace_back operations", "[Vector]") {
    SECTION("Emplace with single argument") {
        Vector<int> vec;
        vec.emplace_back(42);
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == 42);
    }

    SECTION("Emplace with multiple arguments") {
        struct TestStruct {
            int a, b;
            TestStruct(const int x, const int y) : a(x), b(y) {}
            bool operator==(const TestStruct& other) const {
                return a == other.a && b == other.b;
            }
        };

        Vector<TestStruct> vec;
        vec.emplace_back(10, 20);
        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == TestStruct(10, 20));
    }

    SECTION("Emplace strings") {
        Vector<std::string> vec;
        vec.emplace_back("hello");
        vec.emplace_back(5, 'a'); // String constructor with count and character
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "hello");
        REQUIRE(vec[1] == "aaaaa");
    }
}

TEST_CASE("Vector pop_back operations", "[Vector]") {
    Vector<int> vec;

    SECTION("Pop from single element vector") {
        vec.push_back(42);
        REQUIRE(vec.pop_back() == 42);
        REQUIRE(vec.size() == 0);
    }

    SECTION("Pop multiple elements") {
        for (int i = 0; i < 10; ++i) {
            vec.push_back(i);
        }

        REQUIRE(vec.pop_back() == 9);
        REQUIRE(vec.size() == 9);
        REQUIRE(vec.pop_back() == 8);
        REQUIRE(vec.size() == 8);

        // Verify remaining elements are intact
        for (int i = 0; i < 8; ++i) {
            REQUIRE(vec[i] == i);
        }
    }

    SECTION("Pop triggering downward resize") {
        // Fill vector beyond initial capacity
        for (int i = 0; i < 32; ++i) {
            vec.push_back(i);
        }

        // Pop most elements to trigger downward resize
        for (int i = 0; i < 28; ++i) {
            vec.pop_back();
        }

        REQUIRE(vec.size() == 4);
        for (int i = 0; i < 4; ++i) {
            REQUIRE(vec[i] == i);
        }
    }
}

TEST_CASE("Vector operator[] access", "[Vector]") {
    Vector<int> vec;

    SECTION("Access elements") {
        for (int i = 0; i < 10; ++i) {
            vec.push_back(i * 3);
        }

        for (int i = 0; i < 10; ++i) {
            REQUIRE(vec[i] == i * 3);
        }
    }

    SECTION("Modify elements through operator[]") {
        for (int i = 0; i < 5; ++i) {
            vec.push_back(i);
        }

        // Modify elements
        for (int i = 0; i < 5; ++i) {
            vec[i] = i * 10;
        }

        // Verify modifications
        for (int i = 0; i < 5; ++i) {
            REQUIRE(vec[i] == i * 10);
        }
    }

    SECTION("Const access") {
        for (int i = 0; i < 5; ++i) {
            vec.push_back(i);
        }

        const Vector<int>& const_vec = vec;
        for (int i = 0; i < 5; ++i) {
            REQUIRE(const_vec[i] == i);
        }
    }
}

TEST_CASE("Vector with strings", "[Vector]") {
    Vector<std::string> vec;

    SECTION("String operations") {
        vec.push_back(std::string("hello"));
        vec.push_back("world");
        vec.emplace_back("test");

        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == "hello");
        REQUIRE(vec[1] == "world");
        REQUIRE(vec[2] == "test");

        REQUIRE(vec.pop_back() == "test");
        REQUIRE(vec.size() == 2);
    }

    SECTION("String move semantics") {
        std::string str = "movable";
        vec.push_back(std::move(str));
        REQUIRE(vec[0] == "movable");
        // str should be moved from (implementation dependent what state it's in)
    }
}

TEST_CASE("Vector copy constructor and assignment", "[Vector]") {
    SECTION("Copy constructor") {
        Vector<int> original;
        for (int i = 0; i < 10; ++i) {
            original.push_back(i * 2);
        }

        Vector<int> copy(original);
        REQUIRE(copy.size() == original.size());
        for (size_t i = 0; i < copy.size(); ++i) {
            REQUIRE(copy[i] == original[i]);
        }

        // Verify they are independent
        copy[0] = 999;
        REQUIRE(original[0] != 999);
    }

    SECTION("Copy assignment") {
        Vector<int> original;
        for (int i = 0; i < 10; ++i) {
            original.push_back(i * 3);
        }

        Vector<int> copy;
        copy.push_back(42); // Give it some initial content
        copy = original;

        REQUIRE(copy.size() == original.size());
        for (size_t i = 0; i < copy.size(); ++i) {
            REQUIRE(copy[i] == original[i]);
        }

        // Verify they are independent
        copy[0] = 888;
        REQUIRE(original[0] != 888);
    }

    SECTION("Self assignment") {
        Vector<int> vec;
        for (int i = 0; i < 5; ++i) {
            vec.push_back(i);
        }

        // ReSharper disable once CppIdenticalOperandsInBinaryExpression
        vec = vec; // Self assignment

        REQUIRE(vec.size() == 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(vec[i] == i);
        }
    }
}

TEST_CASE("Vector move constructor and assignment", "[Vector]") {
    SECTION("Move constructor") {
        Vector<std::string> original;
        original.push_back("hello");
        original.push_back("world");
        original.push_back("move");

        Vector<std::string> moved(std::move(original));
        REQUIRE(moved.size() == 3);
        REQUIRE(moved[0] == "hello");
        REQUIRE(moved[1] == "world");
        REQUIRE(moved[2] == "move");

        // Original should be in a valid but unspecified state
        REQUIRE(original.size() == 0);
    }

    SECTION("Move assignment") {
        Vector<std::string> original;
        original.push_back("move");
        original.push_back("assignment");

        Vector<std::string> moved;
        moved.push_back("initial"); // Give it some content first
        moved = std::move(original);

        REQUIRE(moved.size() == 2);
        REQUIRE(moved[0] == "move");
        REQUIRE(moved[1] == "assignment");

        // Original should be in a valid but unspecified state
        REQUIRE(original.size() == 0);
    }

    SECTION("Self move assignment") {
        Vector<int> vec;
        for (int i = 0; i < 5; ++i) {
            vec.push_back(i);
        }

        vec = std::move(vec); // Self move assignment

        // Should still be valid (though state is implementation defined)
        REQUIRE_NOTHROW(vec.size());
    }
}

TEST_CASE("Vector resizing behavior", "[Vector]") {
    SECTION("Upward resizing maintains elements") {
        Vector<int> vec;

        // Fill beyond initial capacity multiple times
        for (int i = 0; i < 100; ++i) {
            vec.push_back(i);
        }

        REQUIRE(vec.size() == 100);
        for (int i = 0; i < 100; ++i) {
            REQUIRE(vec[i] == i);
        }
    }

    SECTION("Downward resizing maintains elements") {
        Vector<int> vec;

        // Fill with many elements
        for (int i = 0; i < 100; ++i) {
            vec.push_back(i);
        }

        // Remove most elements
        for (int i = 0; i < 90; ++i) {
            vec.pop_back();
        }

        REQUIRE(vec.size() == 10);
        for (int i = 0; i < 10; ++i) {
            REQUIRE(vec[i] == i);
        }
    }

    SECTION("Resize doesn't happen prematurely") {
        Vector<int> vec;

        // Fill to initial capacity
        for (int i = 0; i < 8; ++i) {
            vec.push_back(i);
        }

        // Remove one element - should not trigger downward resize
        vec.pop_back();
        REQUIRE(vec.size() == 7);

        // Should still be accessible
        for (int i = 0; i < 7; ++i) {
            REQUIRE(vec[i] == i);
        }
    }
}

TEST_CASE("Vector stress tests", "[Vector][Stress]") {
    SECTION("Large number of operations") {
        Vector<int> vec;
        constexpr int operations = 10000;

        // Add many elements
        for (int i = 0; i < operations; ++i) {
            vec.push_back(i);
        }
        REQUIRE(vec.size() == operations);

        // Remove half
        for (int i = 0; i < operations / 2; ++i) {
            vec.pop_back();
        }
        REQUIRE(vec.size() == operations / 2);

        // Verify remaining elements
        for (int i = 0; i < operations / 2; ++i) {
            REQUIRE(vec[i] == i);
        }
    }

    SECTION("Alternating push/pop pattern") {
        Vector<int> vec;

        for (int cycle = 0; cycle < 100; ++cycle) {
            // Push several elements
            for (int i = 0; i < 20; ++i) {
                vec.push_back(cycle * 100 + i);
            }

            // Pop some elements
            for (int i = 0; i < 10; ++i) {
                vec.pop_back();
            }
        }

        // Should still be functional
        REQUIRE(vec.size() == 1000); // 100 * (20 - 10)

        // Test access to verify integrity
        for (size_t i = 0; i < std::min(vec.size(), 50zu); ++i) {
            REQUIRE_NOTHROW(vec[i]);
        }
    }
}

TEST_CASE("Vector edge cases", "[Vector][EdgeCases]") {
    SECTION("Empty vector operations") {
        Vector<int> vec;
        REQUIRE(vec.size() == 0);

        // These should be safe to call on empty vector
        REQUIRE_NOTHROW(vec.size());
    }

    SECTION("Single element operations") {
        Vector<int> vec;
        vec.push_back(42);

        REQUIRE(vec.size() == 1);
        REQUIRE(vec[0] == 42);

        REQUIRE(vec.pop_back() == 42);
        REQUIRE(vec.size() == 0);
    }

    SECTION("Large objects") {
        struct LargeObject {
            std::array<int, 1000> data{};
            int id;

            explicit LargeObject(const int i) : id(i) {
                data.fill(i);
            }

            bool operator==(const LargeObject& other) const {
                return id == other.id;
            }
        };

        Vector<LargeObject> vec;
        vec.emplace_back(42);
        vec.emplace_back(99);

        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == LargeObject(42));
        REQUIRE(vec[1] == LargeObject(99));
    }

    SECTION("Complex object lifecycle") {
        Vector<std::string> vec;

        // Test with complex objects that have their own memory management
        for (int i = 0; i < 100; ++i) {
            vec.push_back(std::string(i + 1, i % 26 + 'a'));
        }

        REQUIRE(vec.size() == 100);

        // Verify content
        for (int i = 0; i < 100; ++i) {
            std::string expected(i + 1, i % 26 + 'a');
            REQUIRE(vec[i] == expected);
        }

        // Test copy and move operations
        Vector<std::string> copy = vec;
        Vector<std::string> moved = std::move(vec);

        REQUIRE(copy.size() == 100);
        REQUIRE(moved.size() == 100);

        for (int i = 0; i < 100; ++i) {
            std::string expected(i + 1, i % 26 + 'a');
            REQUIRE(copy[i] == expected);
            REQUIRE(moved[i] == expected);
        }
    }
}

#include <catch2/catch_test_macros.hpp>
#include "Deque.h"
#include <string>

TEST_CASE("Deque construction", "[Deque]") {
    SECTION("Default construction") {
        const Deque<int> deque;
        REQUIRE(deque.size() == 0);
    }
}

TEST_CASE("Deque push_back operations", "[Deque]") {
    Deque<int> deque;

    SECTION("Push single element") {
        deque.push_back(42);
        REQUIRE(deque.size() == 1);
        REQUIRE(deque[0] == 42);
    }

    SECTION("Push multiple elements") {
        for (int i = 0; i < 10; ++i) {
            deque.push_back(i);
        }
        REQUIRE(deque.size() == 10);
        for (int i = 0; i < 10; ++i) {
            REQUIRE(deque[i] == i);
        }
    }

    SECTION("Push beyond single FixedArray capacity") {
        // Push enough elements to trigger multiple FixedArrays
        constexpr int num_elements = 5000; // More than fixed_arr_n_elem
        for (int i = 0; i < num_elements; ++i) {
            deque.push_back(i);
        }
        REQUIRE(deque.size() == num_elements);
        // Verify all elements are accessible and correct
        for (int i = 0; i < num_elements; ++i) {
            REQUIRE(deque[i] == i);
        }
    }
}

TEST_CASE("Deque push_front operations", "[Deque]") {
    Deque<int> deque;

    SECTION("Push single element to front") {
        deque.push_front(42);
        REQUIRE(deque.size() == 1);
        REQUIRE(deque[0] == 42);
    }

    SECTION("Push multiple elements to front") {
        for (int i = 0; i < 10; ++i) {
            deque.push_front(i);
        }
        REQUIRE(deque.size() == 10);
        // Elements should be in reverse order
        for (int i = 0; i < 10; ++i) {
            REQUIRE(deque[i] == 9 - i);
        }
    }

    SECTION("Mix push_front and push_back") {
        deque.push_back(1);
        deque.push_back(2);
        deque.push_front(0);
        deque.push_front(-1);

        REQUIRE(deque.size() == 4);
        REQUIRE(deque[0] == -1);
        REQUIRE(deque[1] == 0);
        REQUIRE(deque[2] == 1);
        REQUIRE(deque[3] == 2);
    }
}

TEST_CASE("Deque pop operations", "[Deque]") {
    Deque<int> deque;

    SECTION("Pop from back") {
        deque.push_back(1);
        deque.push_back(2);
        deque.push_back(3);

        REQUIRE(deque.pop_back() == 3);
        REQUIRE(deque.size() == 2);
        REQUIRE(deque.pop_back() == 2);
        REQUIRE(deque.size() == 1);
        REQUIRE(deque.pop_back() == 1);
        REQUIRE(deque.size() == 0); // NOLINT
    }

    SECTION("Pop from front") {
        deque.push_back(1);
        deque.push_back(2);
        deque.push_back(3);

        REQUIRE(deque.pop_front() == 1);
        REQUIRE(deque.size() == 2);
        REQUIRE(deque.pop_front() == 2);
        REQUIRE(deque.size() == 1);
        REQUIRE(deque.pop_front() == 3);
        REQUIRE(deque.size() == 0); // NOLINT
    }

    SECTION("Mix pop operations") {
        for (int i = 0; i < 10; ++i) {
            deque.push_back(i);
        }

        REQUIRE(deque.pop_front() == 0);
        REQUIRE(deque.pop_back() == 9);
        REQUIRE(deque.pop_front() == 1);
        REQUIRE(deque.pop_back() == 8);

        REQUIRE(deque.size() == 6);
        REQUIRE(deque[0] == 2);
        REQUIRE(deque[5] == 7);
    }
}

TEST_CASE("Deque operator[] access", "[Deque]") {
    Deque<int> deque;

    SECTION("Random access after mixed operations") {
        // Fill deque with pattern
        for (int i = 0; i < 100; ++i) {
            if (i % 2 == 0) {
                deque.push_back(i);
            } else {
                deque.push_front(-i);
            }
        }

        // Verify all elements are accessible
        for (size_t i = 0; i < deque.size(); ++i) {
            // This should not throw and should return a valid value
            REQUIRE_NOTHROW(deque[i]);
        }
    }

    SECTION("Access across multiple FixedArrays") {
        constexpr int num_elements = 10000;
        for (int i = 0; i < num_elements; ++i) {
            deque.push_back(i * 2); // Use a pattern to verify correctness
        }

        // Test random access across the entire range
        for (int i = 0; i < num_elements; ++i) {
            REQUIRE(deque[i] == i * 2);
        }

        // Test access at boundaries
        REQUIRE(deque[0] == 0);
        REQUIRE(deque[num_elements - 1] == (num_elements - 1) * 2);
    }
}

TEST_CASE("Deque with strings", "[Deque]") {
    SECTION("String operations") {
        Deque<std::string> deque;
        deque.push_back(std::string("hello"));
        deque.push_back("world");
        deque.push_front("hi");

        REQUIRE(deque.size() == 3);
        REQUIRE(deque[0] == "hi");
        REQUIRE(deque[1] == "hello");
        REQUIRE(deque[2] == "world");

        REQUIRE(deque.pop_back() == "world");
        REQUIRE(deque.pop_front() == "hi");
        REQUIRE(deque[0] == "hello");
    }
}

TEST_CASE("Deque stress test", "[Deque][Stress]") {
    Deque<int> deque;

    SECTION("Large scale operations") {
        constexpr int operations = 50000;

        // Add many elements
        for (int i = 0; i < operations; ++i) {
            deque.push_back(i);
        }
        REQUIRE(deque.size() == operations);

        // Remove half from front
        for (int i = 0; i < operations / 2; ++i) {
            REQUIRE(deque.pop_front() == i);
        }
        REQUIRE(deque.size() == operations / 2);

        // Add more to front
        for (int i = 0; i < operations / 4; ++i) {
            deque.push_front(-i - 1);
        }

        // Verify size and some elements
        REQUIRE(deque.size() == operations / 2 + operations / 4);
        REQUIRE(deque[0] == -operations / 4);
        REQUIRE(deque[operations / 4] == operations / 2);
    }

    SECTION("Alternating push/pop pattern") {
        // This tests the dynamic resizing behavior
        for (int cycle = 0; cycle < 1000; ++cycle) {
            // Push a bunch
            for (int i = 0; i < 100; ++i) {
                deque.push_back(cycle * 100 + i);
            }

            // Pop some
            for (int i = 0; i < 50; ++i) {
                deque.pop_front();
            }
        }

        // Should still be functional
        REQUIRE(deque.size() == 50000); // 1000 * (100 - 50)

        // Test random access still works
        for (size_t i = 0; i < std::min(deque.size(), size_t(100)); ++i) {
            REQUIRE_NOTHROW(deque[i]);
        }
    }
}

TEST_CASE("Deque Constructors", "[Deque]") {
    SECTION("Initializer list construction") {
        const Deque deque = {1, 2, 3, 4, 5};
        REQUIRE(deque.size() == 5);
        REQUIRE(deque[0] == 1);
        REQUIRE(deque[1] == 2);
        REQUIRE(deque[2] == 3);
        REQUIRE(deque[3] == 4);
        REQUIRE(deque[4] == 5);
        REQUIRE(deque.front() == 1);
        REQUIRE(deque.back() == 5);
    }

    SECTION("Initializer list with strings") {
        Deque<std::string> deque = {"one", "two", "three"};
        REQUIRE(deque.size() == 3);
        REQUIRE(deque[0] == "one");
        REQUIRE(deque[1] == "two");
        REQUIRE(deque[2] == "three");
    }
}

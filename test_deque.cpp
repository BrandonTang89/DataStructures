#include <catch2/catch_test_macros.hpp>
#include "Deque.h"
#include <string>
#include <vector>

TEST_CASE("Deque construction", "[Deque]") {
    SECTION("Default construction") {
        const Deque<int> deque;
        REQUIRE(deque.size() == 0); // NOLINT
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
        for (size_t i = 0; i < deque.size(); ++i) { // NOLINT
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
        for (size_t i = 0; i < std::min(deque.size(), 100zu); ++i) {
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

TEST_CASE("Deque Iterator operations", "[Deque][Iterator]") {
    SECTION("Basic iterator functionality") {
        Deque deque = {1, 2, 3, 4, 5};

        // Test begin() and end()
        auto it = deque.begin();
        auto end_it = deque.end();

        REQUIRE(*it == 1);

        // Test dereference operator
        REQUIRE(*it == 1);
        *it = 10;
        REQUIRE(*it == 10);
        REQUIRE(deque[0] == 10);
        REQUIRE(*(end_it.prev()) == 5); // Should point to last element
    }

    SECTION("Iterator increment operations") {
        Deque deque = {1, 2, 3, 4, 5};
        auto it = deque.begin();

        // Test prefix increment
        REQUIRE(*it == 1);
        ++it;
        REQUIRE(*it == 2);
        ++it;
        REQUIRE(*it == 3);

        // Test postfix increment
        auto old_it = it++;
        REQUIRE(*old_it == 3);
        REQUIRE(*it == 4);
    }

    SECTION("Iterator decrement operations") {
        Deque deque = {1, 2, 3, 4, 5};
        auto it = deque.begin();

        // Move to middle
        ++it;
        ++it;
        REQUIRE(*it == 3);

        // Test prefix decrement
        --it;
        REQUIRE(*it == 2);
        --it;
        REQUIRE(*it == 1);

        // Test postfix decrement
        ++it;
        ++it;
        REQUIRE(*it == 3);
        auto old_it = it--;
        REQUIRE(*old_it == 3);
        REQUIRE(*it == 2);
    }

    SECTION("Iterator traversal") {
        Deque<int> deque;
        for (int i = 0; i < 10; ++i) {
            deque.push_back(i);
        }

        // Forward traversal
        int expected = 0;
        for (auto it = deque.begin(); it.current_array != nullptr; ++it) {
            REQUIRE(*it == expected);
            ++expected;
        }
        REQUIRE(expected == 10);

        // Manual traversal using next()
        auto it = deque.begin();
        for (int i = 0; i < 10; ++i) {
            REQUIRE(*it == i);
            it = it.next();
        }
        REQUIRE(it.current_array == nullptr); // Should be at end
    }

    SECTION("Iterator with large deque across multiple FixedArrays") {
        Deque<int> deque;
        constexpr int num_elements = 10000; // Ensure multiple FixedArrays

        for (int i = 0; i < num_elements; ++i) {
            deque.push_back(i);
        }

        // Test forward iteration through all elements
        int count = 0;
        for (auto it = deque.begin(); it.current_array != nullptr; ++it) {
            REQUIRE(*it == count);
            ++count;
        }
        REQUIRE(count == num_elements);

        // Test that we can modify elements through iterator
        auto it = deque.begin();
        for (int i = 0; i < 100; ++i) {
            *it = i * 10;
            ++it;
        }

        // Verify modifications
        for (int i = 0; i < 100; ++i) {
            REQUIRE(deque[i] == i * 10);
        }
    }

    SECTION("Iterator with mixed push operations") {
        Deque<int> deque;

        // Create a pattern: push_front some, push_back some
        for (int i = 0; i < 5; ++i) {
            deque.push_front(-i - 1); // -1, -2, -3, -4, -5
            deque.push_back(i + 1); //  1,  2,  3,  4,  5
        }

        // Expected order: [-5, -4, -3, -2, -1, 1, 2, 3, 4, 5]
        std::vector expected = {-5, -4, -3, -2, -1, 1, 2, 3, 4, 5};

        int index = 0;
        for (auto it = deque.begin(); it.current_array != nullptr; ++it) {
            REQUIRE(*it == expected[index]);
            ++index;
        }
        REQUIRE(index == expected.size());
    }

    SECTION("Range-based for loop") {
        Deque<std::string> deque = {"hello", "world", "test"};

        std::vector<std::string> collected;
        for (const auto &item: deque) {
            collected.push_back(item);
        }

        REQUIRE(collected.size() == 3);
        REQUIRE(collected[0] == "hello");
        REQUIRE(collected[1] == "world");
        REQUIRE(collected[2] == "test");
    }

    SECTION("Empty deque iterator") {
        Deque<int> deque;

        auto begin_it = deque.begin();
        auto end_it = deque.end();

        // For empty deque, begin should point to end immediately
        REQUIRE(begin_it.current_array == nullptr); // Points to the initial FixedArray
        REQUIRE(end_it.current_array == nullptr);
        ++begin_it;
        REQUIRE(begin_it.current_array == nullptr);
    }

    SECTION("Iterator prev() method") {
        Deque deque = {1, 2, 3, 4, 5};

        // Start from end and work backwards using prev()
        auto it = deque.end().prev();
        REQUIRE(*it == 5);

        it = it.prev();
        REQUIRE(*it == 4);

        it = it.prev();
        REQUIRE(*it == 3);

        it = it.prev();
        REQUIRE(*it == 2);

        it = it.prev();
        REQUIRE(*it == 1);
    }

    SECTION("Iterator boundary conditions") {
        Deque<int> deque;

        // Test with exactly one element
        deque.push_back(42);
        auto it = deque.begin();
        REQUIRE(*it == 42);

        ++it;
        REQUIRE(it.current_array == nullptr); // Should be at end

        // Test prev from end
        it = deque.end().prev();
        REQUIRE(*it == 42);
    }
}

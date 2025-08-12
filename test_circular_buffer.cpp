#include <catch2/catch_test_macros.hpp>
#include "CircularBuffer.h"
#include "TestingTracker.h"

TEST_CASE("CircularBuffer construction", "[CircularBuffer]") {
    SECTION("Create buffer with capacity") {
        CircularBuffer<int> buffer{100};
        REQUIRE(true); // Constructor should not throw
    }
}

TEST_CASE("CircularBuffer push_back", "[CircularBuffer]") {
    CircularBuffer<int> buffer(3);

    SECTION("Push elements within capacity") {
        REQUIRE_NOTHROW(buffer.push_back(1));
        REQUIRE_NOTHROW(buffer.push_back(2));
        REQUIRE_NOTHROW(buffer.push_back(3));
        REQUIRE(buffer.size() == 3);
    }

    SECTION("Push beyond capacity should resize") {
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);
        buffer.push_back(4);

        REQUIRE(buffer.size() == 4);
    }
}

TEST_CASE("CircularBuffer with strings", "[CircularBuffer]") {
    CircularBuffer<std::string> buffer(2);

    SECTION("Push string elements") {
        REQUIRE_NOTHROW(buffer.push_back("hello"));
        REQUIRE_NOTHROW(buffer.push_back("world"));
        buffer.pop_back();
        buffer.pop_back();
    }
}

TEST_CASE("CircularBuffer resizing and operator[]", "[CircularBuffer]") {
    CircularBuffer<int> buffer(4);

    SECTION("Upward resizing maintains order and size") {
        for (int i = 0; i < 20; ++i) {
            buffer.push_back(i);
        }
        REQUIRE(buffer.size() == 20);
        for (int i = 0; i < 20; ++i) {
            REQUIRE(buffer[i] == i);
        }
        REQUIRE(buffer.capacity() >= 20);
    }

    SECTION("Downward resizing maintains order and size") {
        for (int i = 0; i < 32; ++i) {
            buffer.push_back(i);
        }
        for (int i = 0; i < 28; ++i) {
            buffer.pop_front();
        }
        REQUIRE(buffer.size() == 4);
        for (int i = 0; i < 4; ++i) {
            REQUIRE(buffer[i] == 28 + i);
        }
        REQUIRE(buffer.capacity() <= 32);
    }

    SECTION("operator[] works after wraparound and resize") {
        for (int i = 0; i < 8; ++i) {
            buffer.push_back(i);
        }
        for (int i = 0; i < 4; ++i) {
            buffer.pop_front();
        }
        for (int i = 8; i < 12; ++i) {
            buffer.push_back(i);
        }
        // Buffer now contains 4..11
        for (int i = 0; i < buffer.size(); ++i) {
            REQUIRE(buffer[i] == 4 + i);
        }
    }
}

TEST_CASE("CircularBuffer robustness tests", "[CircularBuffer][Robustness]") {
    SECTION("Multiple upward resizes maintain correctness") {
        CircularBuffer<int> buffer(8);

        // Fill buffer to trigger multiple resizes
        for (int i = 0; i < 1000; ++i) {
            buffer.push_back(i);
        }

        REQUIRE(buffer.size() == 1000);
        REQUIRE(buffer.capacity() >= 1000);

        // Verify all elements are correct and accessible
        for (int i = 0; i < 1000; ++i) {
            REQUIRE(buffer[i] == i);
        }

        // Verify front and back work correctly
        REQUIRE(buffer.front() == 0);
        REQUIRE(buffer.back() == 999);
    }

    SECTION("Multiple downward resizes maintain correctness") {
        CircularBuffer<int> buffer(8);

        // Fill buffer with many elements
        for (int i = 0; i < 1000; ++i) {
            buffer.push_back(i);
        }

        // Remove most elements to trigger downward resizing
        for (int i = 0; i < 990; ++i) {
            buffer.pop_front();
        }

        REQUIRE(buffer.size() == 10);
        // Capacity should have decreased significantly
        REQUIRE(buffer.capacity() < 1000);

        // Verify remaining elements are correct
        for (int i = 0; i < 10; ++i) {
            REQUIRE(buffer[i] == 990 + i);
        }

        REQUIRE(buffer.front() == 990);
        REQUIRE(buffer.back() == 999);
    }

    SECTION("Mixed operations with complex wraparound") {
        CircularBuffer<int> buffer(16);

        // Create complex pattern that causes wraparound
        for (int cycle = 0; cycle < 20; ++cycle) {
            // Add elements
            for (int i = 0; i < 30; ++i) {
                buffer.push_back(cycle * 100 + i);
            }

            // Remove some from front
            for (int i = 0; i < 15; ++i) {
                buffer.pop_front();
            }

            // Add some to front
            for (int i = 0; i < 5; ++i) {
                buffer.push_front(-(cycle * 100 + i + 1));
            }

            // Remove some from back
            for (int i = 0; i < 10; ++i) {
                buffer.pop_back();
            }
        }

        // Buffer should still be functional
        REQUIRE(buffer.size() > 0);

        // operator[] should work for all valid indices
        for (size_t i = 0; i < buffer.size(); ++i) {
            REQUIRE_NOTHROW(buffer[i]);
        }

        // front() and back() should work
        REQUIRE_NOTHROW(buffer.front());
        REQUIRE_NOTHROW(buffer.back());
    }

    SECTION("Size is maintained correctly through all operations") {
        CircularBuffer<int> buffer(4);
        size_t expected_size = 0;

        // Push back operations
        for (int i = 0; i < 50; ++i) {
            buffer.push_back(i);
            ++expected_size;
            REQUIRE(buffer.size() == expected_size);
        }

        // Push front operations
        for (int i = 0; i < 30; ++i) {
            buffer.push_front(-i);
            ++expected_size;
            REQUIRE(buffer.size() == expected_size);
        }

        // Pop back operations
        for (int i = 0; i < 25; ++i) {
            buffer.pop_back();
            --expected_size;
            REQUIRE(buffer.size() == expected_size);
        }

        // Pop front operations
        for (int i = 0; i < 25; ++i) {
            buffer.pop_front();
            --expected_size;
            REQUIRE(buffer.size() == expected_size);
        }

        REQUIRE(expected_size == 30);
        REQUIRE(buffer.size() == 30);
    }

    SECTION("operator[] correctness after resize operations") {
        CircularBuffer<int> buffer(4);

        // Fill with pattern
        for (int i = 0; i < 20; ++i) {
            buffer.push_back(i * 3); // 0, 3, 6, 9, 12, ...
        }

        // Verify pattern after upward resize
        for (int i = 0; i < 20; ++i) {
            REQUIRE(buffer[i] == i * 3);
        }

        // Remove elements from front to trigger potential downward resize
        for (int i = 0; i < 15; ++i) {
            buffer.pop_front();
        }

        // Verify remaining elements are still correct
        REQUIRE(buffer.size() == 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(buffer[i] == (15 + i) * 3); // Should be 45, 48, 51, 54, 57
        }
    }
}

TEST_CASE("CircularBuffer element construction/destruction count", "[CircularBuffer][Lifetime]") {
    TestingTracker::constructed = 0;
    TestingTracker::destructed = 0;

    constexpr int num_elements = 50; {
        CircularBuffer<TestingTracker> buffer(8);
        for (int i = 0; i < num_elements; ++i) {
            buffer.emplace_back();
        }

        REQUIRE(TestingTracker::constructed == num_elements);
    }

    REQUIRE(TestingTracker::destructed == num_elements);
}

TEST_CASE("CircularBuffer Iterator functionality", "[CircularBuffer][Iterator]") {
    SECTION("Basic iterator operations") {
        CircularBuffer<int> buffer(4);
        buffer.push_back(10);
        buffer.push_back(20);
        buffer.push_back(30);

        auto it = buffer.begin();
        REQUIRE(*it == 10);
        ++it;
        REQUIRE(*it == 20);
        ++it;
        REQUIRE(*it == 30);
        ++it;
        REQUIRE(it == buffer.end());
    }

    SECTION("Iterator dereference and modification") {
        CircularBuffer<int> buffer(3);
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);

        auto it = buffer.begin();
        *it = 100;
        REQUIRE(buffer[0] == 100);
        REQUIRE(*it == 100);

        ++it;
        *it = 200;
        REQUIRE(buffer[1] == 200);
        REQUIRE(*it == 200);
    }

    SECTION("Range-based for loop") {
        CircularBuffer<int> buffer(5);
        for (int i = 1; i <= 5; ++i) {
            buffer.push_back(i * 10);
        }

        std::vector<int> collected;
        for (const auto& value : buffer) {
            collected.push_back(value);
        }

        REQUIRE(collected.size() == 5);
        for (size_t i = 0; i < 5; ++i) {
            REQUIRE(collected[i] == (i + 1) * 10);
        }
    }

    SECTION("Range-based for loop with modification") {
        CircularBuffer<int> buffer(3);
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);

        for (auto& value : buffer) {
            value *= 2;
        }

        REQUIRE(buffer[0] == 2);
        REQUIRE(buffer[1] == 4);
        REQUIRE(buffer[2] == 6);
    }

    SECTION("Iterator with wraparound") {
        CircularBuffer<int> buffer(4);

        // Fill buffer
        for (int i = 0; i < 4; ++i) {
            buffer.push_back(i);
        }

        // Remove from front and add to back to cause wraparound
        buffer.pop_front(); // Remove 0
        buffer.pop_front(); // Remove 1
        buffer.push_back(4); // Add 4
        buffer.push_back(5); // Add 5

        // Buffer now contains [2, 3, 4, 5] with wraparound
        std::vector<int> expected = {2, 3, 4, 5};
        std::vector<int> actual;

        for (const auto& value : buffer) {
            actual.push_back(value);
        }

        REQUIRE(actual == expected);
    }

    SECTION("Empty buffer iteration") {
        CircularBuffer<int> buffer(4);

        auto begin_it = buffer.begin();
        auto end_it = buffer.end();

        REQUIRE(begin_it == end_it);

        // Range-based for loop should not execute
        int count = 0;
        for (const auto& value : buffer) {
            ++count;
            (void)value; // Suppress unused variable warning
        }
        REQUIRE(count == 0);
    }

    SECTION("Single element iteration") {
        CircularBuffer<int> buffer(4);
        buffer.push_back(42);

        int count = 0;
        for (const auto& value : buffer) {
            REQUIRE(value == 42);
            ++count;
        }
        REQUIRE(count == 1);
    }
}

TEST_CASE("CircularBuffer Iterator arithmetic operations", "[CircularBuffer][Iterator]") {
    SECTION("Iterator increment and decrement") {
        CircularBuffer<int> buffer(5);
        for (int i = 0; i < 5; ++i) {
            buffer.push_back(i * 10);
        }

        auto it = buffer.begin();

        // Test prefix increment
        REQUIRE(*it == 0);
        REQUIRE(*(++it) == 10);
        REQUIRE(*it == 10);

        // Test postfix increment
        REQUIRE(*(it++) == 10);
        REQUIRE(*it == 20);

        // Test prefix decrement
        REQUIRE(*(--it) == 10);
        REQUIRE(*it == 10);

        // Test postfix decrement
        REQUIRE(*(it--) == 10);
        REQUIRE(*it == 0);
    }

    SECTION("Iterator addition and subtraction") {
        CircularBuffer<int> buffer(5);
        for (int i = 0; i < 5; ++i) {
            buffer.push_back(i * 10);
        }

        auto it = buffer.begin();

        // Test addition
        auto it2 = it + 2;
        REQUIRE(*it2 == 20);

        auto it3 = it + 4;
        REQUIRE(*it3 == 40);

        // Test subtraction
        auto it4 = it3 - 2;
        REQUIRE(*it4 == 20);

        // Test compound assignment
        it += 3;
        REQUIRE(*it == 30);

        it -= 1;
        REQUIRE(*it == 20);
    }

    SECTION("Iterator comparison operations") {
        CircularBuffer<int> buffer(5);
        for (int i = 0; i < 5; ++i) {
            buffer.push_back(i);
        }

        auto it1 = buffer.begin();
        auto it2 = buffer.begin() + 2;
        auto it3 = buffer.end();

        REQUIRE(it1 < it2);
        REQUIRE(it2 > it1);
        REQUIRE(it1 <= it2);
        REQUIRE(it2 >= it1);
        REQUIRE(it1 != it2);
        REQUIRE(it1 == buffer.begin());

        REQUIRE(it2 < it3);
        REQUIRE(it3 > it2);
    }

    SECTION("Iterator distance") {
        CircularBuffer<int> buffer(5);
        for (int i = 0; i < 5; ++i) {
            buffer.push_back(i);
        }

        auto it1 = buffer.begin();
        auto it2 = buffer.begin() + 3;
        auto it3 = buffer.end();

        REQUIRE(it2 - it1 == 3);
        REQUIRE(it3 - it1 == 5);
        REQUIRE(it3 - it2 == 2);
    }
}

TEST_CASE("CircularBuffer Const Iterator functionality", "[CircularBuffer][ConstIterator]") {
    SECTION("Const iterator basic operations") {
        CircularBuffer<int> buffer(3);
        buffer.push_back(10);
        buffer.push_back(20);
        buffer.push_back(30);

        const auto& const_buffer = buffer;

        auto cit = const_buffer.begin();
        REQUIRE(*cit == 10);
        ++cit;
        REQUIRE(*cit == 20);
        ++cit;
        REQUIRE(*cit == 30);
        ++cit;
        REQUIRE(cit == const_buffer.end());
    }

    SECTION("Const iterator range-based for loop") {
        CircularBuffer<std::string> buffer(3);
        buffer.push_back("hello");
        buffer.push_back("world");
        buffer.push_back("test");

        const auto& const_buffer = buffer;

        std::vector<std::string> collected;
        for (const auto& value : const_buffer) {
            collected.push_back(value);
        }

        REQUIRE(collected.size() == 3);
        REQUIRE(collected[0] == "hello");
        REQUIRE(collected[1] == "world");
        REQUIRE(collected[2] == "test");
    }

    SECTION("cbegin and cend methods") {
        CircularBuffer<int> buffer(3);
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);

        auto cit = buffer.cbegin();
        REQUIRE(*cit == 1);
        ++cit;
        REQUIRE(*cit == 2);
        ++cit;
        REQUIRE(*cit == 3);
        ++cit;
        REQUIRE(cit == buffer.cend());
    }
}

TEST_CASE("CircularBuffer Iterator with complex operations", "[CircularBuffer][Iterator]") {
    SECTION("Iterator after buffer resize") {
        CircularBuffer<int> buffer(4);

        // Fill buffer to capacity
        for (int i = 0; i < 4; ++i) {
            buffer.push_back(i);
        }

        // Add one more to trigger resize
        buffer.push_back(4);

        // Verify iteration works after resize
        std::vector<int> expected = {0, 1, 2, 3, 4};
        std::vector<int> actual;

        for (const auto& value : buffer) {
            actual.push_back(value);
        }

        REQUIRE(actual == expected);
    }

    SECTION("Iterator with pop operations") {
        CircularBuffer<int> buffer(6);

        // Fill buffer
        for (int i = 0; i < 6; ++i) {
            buffer.push_back(i * 10);
        }

        // Remove some elements
        buffer.pop_front(); // Remove 0
        buffer.pop_back();  // Remove 50

        // Verify iteration works correctly
        std::vector<int> expected = {10, 20, 30, 40};
        std::vector<int> actual;

        for (const auto& value : buffer) {
            actual.push_back(value);
        }

        REQUIRE(actual == expected);
    }

    SECTION("Iterator with mixed push/pop causing wraparound") {
        CircularBuffer<int> buffer(4);

        // Fill buffer
        for (int i = 0; i < 4; ++i) {
            buffer.push_back(i);
        }

        // Create wraparound scenario
        buffer.pop_front(); // Remove 0
        buffer.pop_front(); // Remove 1
        buffer.push_back(4); // Add 4
        buffer.push_back(5); // Add 5 (triggers resize)

        // Buffer should now contain [2, 3, 4, 5]
        std::vector<int> expected = {2, 3, 4, 5};
        std::vector<int> actual;

        for (const auto& value : buffer) {
            actual.push_back(value);
        }

        REQUIRE(actual == expected);
    }

    SECTION("Iterator pointer access") {
        struct TestStruct {
            int value;
            TestStruct(int v) : value(v) {}
        };

        CircularBuffer<TestStruct> buffer(3);
        buffer.emplace_back(10);
        buffer.emplace_back(20);
        buffer.emplace_back(30);

        auto it = buffer.begin();
        REQUIRE(it->value == 10);
        ++it;
        REQUIRE(it->value == 20);
        ++it;
        REQUIRE(it->value == 30);
    }
}

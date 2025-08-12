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

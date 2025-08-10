#include <catch2/catch_test_macros.hpp>
#include "CircularBuffer.h"

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

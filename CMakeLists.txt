cmake_minimum_required(VERSION 3.31.6)
project(DataStructures)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address,undefined")

find_package(Catch2)

add_executable(DataStructures main.cpp
        CachingAllocator.h
        WideSegmentTree.h
        Vector.h
        test_vector.cpp)

add_executable(tests
        test_circular_buffer.cpp
        test_deque.cpp
        test_vector.cpp
        TestingTracker.h
        TestingTracker.cpp
        WideSegmentTree.h)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
enable_testing()
add_test(NAME CircularBufferTests COMMAND tests)
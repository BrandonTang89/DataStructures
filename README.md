# Data Structures

Collection of arbitrary data structures implemented in C++.

- Circular Buffer
    - A dynamically resizing array that allows for efficient insertion and deletion of elements at both ends
    - Allows for O(1) random access
- Caching Allocator
    - An abstraction useful for frequent new/delete operations on some type
    - We still call the constructor and destructor of the type but cache the allocated memory
- Deque
    - A double-ended queue that allows for efficient insertion and deletion of elements at both ends
    - Allows for O(1) random access
    - Prevents iterator invalidation by storing elements in allocated fixed length arrays that never move
    - Internally relies on a circular buffer to store pointers to the fixed length arrays that are dynamically allocated
      and managed by a caching allocator
- TestingTracker
  - A utility class to track the number of times a class has objects constructed and destructed

To do (deque):

- const iterators

To do (General):

- Benchmarking
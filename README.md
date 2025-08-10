# Data Structures
Collection of arbitrary data structures implemented in C++.

- Circular Buffer
  - A dynamically resizing array that allows for efficient insertion and deletion of elements at both ends
  - Allows for O(1) random access
- Deque
  - A double-ended queue that allows for efficient insertion and deletion of elements at both ends
  - Allows for O(1) random access
  - Prevents iterator invalidation by storing elements in allocated fixed length arrays that never move
  - Internally relies on a circular buffer to store pointers to the fixed length arrays

To do (deque):
- Ensure deque calls the relevant constructors and deconstructors properly, with std:;string tests
- Caching of memory allocations and deallocations for the fixed length arrays
- Iterators

To do (General):
- Benchmarking
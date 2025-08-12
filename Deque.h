/// A Double Ended Queue (Deque)

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>

#include "CircularBuffer.h"
#include "CachingAllocator.h"

template<typename T>
class Deque {
    static constexpr size_t fixed_arr_n_elem = std::max((4096 + sizeof(T) - 1) / sizeof(T), 16zu);

    struct alignas(4096) FixedArray {
        // Page aligned
        std::array<std::byte, fixed_arr_n_elem * sizeof(T)> data_storage{};
        T *data{reinterpret_cast<T *>(data_storage.data())};
        FixedArray *next{nullptr};
        FixedArray *prev{nullptr};

        template<class Self>
        auto &&operator[](this Self &&self, size_t index) {
            assert(index < fixed_arr_n_elem && "Index out of bounds");
            return self.data[index];
        }
    };

    CachingAllocator<FixedArray> allocator{5};
    CircularBuffer<std::unique_ptr<FixedArray, CachingAllocator<FixedArray> &> > buffer{};
    // Implicitly, these are on the first and last FixedArray, which may be the same
    size_t first_idx = 0; // inv: 0 <= first_idx < fixed_arr_n_elem
    size_t last_idx = 0; // inv: 0 < last_idx <= fixed_arr_n_elem
    size_t size_{0};

    template<typename... Args>
    void emplace_front_impl(Args &&... value) {
        if (first_idx == 0) [[unlikely]] {
            FixedArray *first_array_ptr = buffer.front().get();
            buffer.push_front(allocator.allocate());
            buffer.front()->next = first_array_ptr;
            first_array_ptr->prev = buffer.front().get();
            first_idx = fixed_arr_n_elem;
        }
        FixedArray &first_array = *buffer.front();
        --first_idx;
        new(&first_array[first_idx]) T(std::forward<Args>(value)...);
        ++size_;
    }

    template<typename... Args>
    void emplace_back_impl(Args &&... value) {
        if (last_idx == fixed_arr_n_elem) [[unlikely]] {
            FixedArray *last_array_ptr = buffer.back().get();
            buffer.push_back(allocator.allocate());
            buffer.back()->prev = last_array_ptr;
            last_array_ptr->next = buffer.back().get();
            last_idx = 0;
        }
        FixedArray &last_array = *buffer.back();
        new(&last_array[last_idx]) T(std::forward<Args>(value)...);
        ++last_idx;
        ++size_;
    }

    struct Iterator {
        Deque *deque{};
        FixedArray *current_array{};
        size_t index{};

        T &operator*() {
            return current_array->data[index];
        }

        [[nodiscard]] Iterator next() const {
            if (current_array == nullptr) [[unlikely]] {
                return *this; // Already at end
            }
            if (index == deque->last_idx - 1 && current_array == deque->buffer.back().get()) [[unlikely]] {
                return Iterator{deque, nullptr, 0}; // Move to end
            }

            if (index == fixed_arr_n_elem - 1) {
                return Iterator{deque, current_array->next, 0}; // Move to next array
            }


            return Iterator{deque, current_array, index + 1};
        }

        [[nodiscard]] Iterator prev() const {
            if (current_array == nullptr) {
                // Coming from end, go to last valid element
                return Iterator{deque, deque->buffer.back().get(), deque->last_idx - 1};
            }

            // Check if we're at the first element of the deque
            if (current_array == deque->buffer.front().get() && index == deque->first_idx) {
                return Iterator{deque, nullptr, 0}; // Before begin (invalid)
            }

            // Move to previous position in current array
            if (index > 0) {
                return Iterator{deque, current_array, index - 1};
            }

            // Move to previous array
            return Iterator{deque, current_array->prev, fixed_arr_n_elem - 1};
        }

        Iterator &operator++() {
            *this = next();
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        Iterator &operator--() {
            *this = prev();
            return *this;
        }

        Iterator operator--(int) {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        // Equality operators needed for range-based for loops
        bool operator==(const Iterator &other) const {
            return deque == other.deque &&
                   current_array == other.current_array &&
                   index == other.index;
        }

        bool operator!=(const Iterator &other) const {
            return !(*this == other);
        }
    };

public:
    Deque() {
        buffer.push_back(allocator.allocate());
    }

    Deque(std::initializer_list<T> init) {
        buffer.push_back(allocator.allocate());
        for (const auto &value: init) {
            emplace_back_impl(value);
        }
    }

    // Rule of 5 Methods
    ~Deque() {
        for (T &item: *this) {
            item.~T(); // Explicitly call destructor for each element
        }
    }

    Deque(const Deque &other) = delete;

    Deque &operator=(const Deque &other) = delete;

    Deque(Deque &&other) noexcept
        : allocator(std::move(other.allocator)), buffer(std::move(other.buffer)),
          first_idx(other.first_idx), last_idx(other.last_idx), size_(other.size_) {
        other.first_idx = 0;
        other.last_idx = 0;
        other.size_ = 0;
    }

    Deque &operator=(Deque &&other) noexcept {
        if (this != &other) {
            for (T &item: *this) {
                item.~T();
            }
            new(this) Deque(std::move(other));
        }
        return *this;
    }


    void push_back(const T &value) requires std::copy_constructible<T> {
        emplace_back_impl(value);
    }

    void push_back(T &&value) requires std::move_constructible<T> {
        emplace_back_impl(std::move(value));
    }

    template<typename... Args>
    void emplace_back(Args &&... args) requires std::constructible_from<T, Args...> {
        emplace_back_impl(std::forward<Args>(args)...);
    }

    void push_front(const T &value) requires std::copy_constructible<T> {
        emplace_front_impl(value);
    }

    void push_front(T &&value) requires std::move_constructible<T> {
        emplace_front_impl(std::move(value));
    }

    template<typename... Args>
    void emplace_front(Args &&... args) requires std::constructible_from<T, Args...> {
        emplace_front_impl(std::forward<Args>(args)...);
    }

    T pop_back() {
        assert(size_ > 0 && "Cannot pop from an empty deque");
        --size_;
        --last_idx;
        FixedArray &last_array = *buffer.back();
        T value = std::move(last_array[last_idx]);
        if (last_idx == 0) [[unlikely]] {
            // Remove the last FixedArray if it is empty
            buffer.pop_back();
            last_idx = fixed_arr_n_elem;
        }
        return value;
    }

    T pop_front() {
        assert(size_ > 0 && "Cannot pop from an empty deque");
        --size_;
        FixedArray &first_array = *buffer.front();
        T value = std::move(first_array[first_idx]);
        ++first_idx;
        if (first_idx == fixed_arr_n_elem) [[unlikely]] {
            // Remove the first FixedArray if it is empty
            buffer.pop_front();
            first_idx = 0; // Reset to the start of the next FixedArray
        }
        return value;
    }

    template<class Self>
    auto &&operator[](this Self &&self, const size_t index) {
        assert(index < self.size_ && "Index out of bounds");

        const size_t first_size = fixed_arr_n_elem - self.first_idx;
        if (index < first_size) {
            // Access within the first FixedArray
            return (*self.buffer.front())[self.first_idx + index];
        }

        const size_t adjusted_index = index - first_size;
        const size_t num_full_arrays = adjusted_index / fixed_arr_n_elem;
        const size_t index_in_array = adjusted_index % fixed_arr_n_elem;
        return (*self.buffer[num_full_arrays + 1])[index_in_array];
    }

    template<typename Self>
    auto &&back(this Self &&self) {
        assert(self.size_ > 0 && "Deque is empty");
        return (*self.buffer.back())[self.last_idx - 1];
    }

    template<typename Self>
    auto &&front(this Self &&self) {
        assert(self.size_ > 0 && "Deque is empty");
        return (*self.buffer.front())[self.first_idx];
    }

    Iterator end() {
        return Iterator{this, nullptr, 0}; // End of iteration
    }

    Iterator begin() {
        if (size_ == 0) [[unlikely]] {
            return end(); // Empty deque
        }
        return Iterator{this, buffer.front().get(), first_idx};
    }

    [[nodiscard]] size_t size() const {
        return size_;
    }

    [[nodiscard]] bool empty() const {
        return size_ == 0;
    }
};

// Explicit template instantiations for common types
template class Deque<int>;
template class Deque<std::string>;

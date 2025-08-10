/// A Double Ended Queue (Deque)

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>

#include "CircularBuffer.h"

template<typename T>
class Deque {
    class Iterator {
    };

    static constexpr size_t fixed_arr_n_elem = std::max((4096 + sizeof(T) - 1) / sizeof(T), 16zu);

    struct FixedArray {
        // page aligned
        alignas(4096) std::array<std::byte, fixed_arr_n_elem * sizeof(T)> data_storage;
        T *data{reinterpret_cast<T *>(data_storage.data())};
        FixedArray *next{nullptr};
        FixedArray *prev{nullptr};

        template<class Self>
        auto&& operator[](this Self&& self, size_t index) {
            assert(index < fixed_arr_n_elem && "Index out of bounds");
            return self.data[index];
        }
    };

    CircularBuffer<std::unique_ptr<FixedArray> > buffer{};

    // Implicitly, these are on the first and last FixedArray, which may be the same
    size_t first_idx = 0; // inv: 0 <= first_idx < fixed_arr_n_elem
    size_t last_idx = 0; // inv: 0 < last_idx <= fixed_arr_n_elem
    size_t size_{0};

    template<typename... Args>
    void emplace_front_impl(Args &&... value) {
        if (first_idx == 0) [[unlikely]] {
            buffer.push_front(std::make_unique<FixedArray>());
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
            buffer.push_back(std::make_unique<FixedArray>());
            last_idx = 0;
        }
        FixedArray &last_array = *buffer.back();
        new(&last_array[last_idx]) T(std::forward<Args>(value)...);
        ++last_idx;
        ++size_;
    }

public:
    Deque() {
        buffer.push_back(std::make_unique<FixedArray>());
    }

    Deque(std::initializer_list<T> init) {
        buffer.push_back(std::make_unique<FixedArray>());
        for (const auto &value : init) {
            emplace_back_impl(value);
        }
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

    template <class Self>
    auto&& operator[](this Self&& self, const size_t index) {
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
    auto&& back(this Self&& self) {
        assert(self.size_ > 0 && "Deque is empty");
        return (*self.buffer.back())[self.last_idx - 1];
    }

    template<typename Self>
    auto&& front(this Self&& self) {
        assert(self.size_ > 0 && "Deque is empty");
        return (*self.buffer.front())[self.first_idx];
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

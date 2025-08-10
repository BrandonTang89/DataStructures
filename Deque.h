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

        T &operator[](size_t index) {
            assert(index < fixed_arr_n_elem && "Index out of bounds");
            return data[index];
        }
    };

    CircularBuffer<std::unique_ptr<FixedArray> > buffer{};

    // Implicitly, these are on the first and last FixedArray, which may be the same
    size_t first_idx = 0; // inv: 0 <= first_idx < fixed_arr_n_elem
    size_t last_idx = 0; // inv: 0 < last_idx <= fixed_arr_n_elem
    size_t size_{0};

    template<typename ValueType> requires std::same_as<std::remove_cvref_t<ValueType>, T>
    void push_front_impl(ValueType &&value) {
        if (first_idx == 0) [[unlikely]] {
            buffer.push_front(std::make_unique<FixedArray>());
            first_idx = fixed_arr_n_elem;
        }
        FixedArray &first_array = *buffer.front();
        --first_idx;
        new(&first_array[first_idx]) T(std::forward<ValueType>(value));
        ++size_;
    }

    template<typename ValueType> requires std::same_as<std::remove_cvref_t<ValueType>, T>
    void push_back_impl(ValueType &&value) {
        if (last_idx == fixed_arr_n_elem) [[unlikely]] {
            buffer.push_back(std::make_unique<FixedArray>());
            last_idx = 0;
        }
        FixedArray &last_array = *buffer.back();
        new(&last_array[last_idx]) T(std::forward<ValueType>(value));
        ++last_idx;
        ++size_;
    }

public:
    Deque() {
        buffer.push_back(std::make_unique<FixedArray>());
    }

    void push_back(const T &value) {
        push_back_impl(value);
    }

    void push_back(T &&value) {
        push_back_impl(std::move(value));
    }

    void push_front(const T &value) {
        push_front_impl(value);
    }

    void push_front(T &&value) {
        push_front_impl(std::move(value));
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

    T &operator[](const size_t index) {
        assert(index < size_ && "Index out of bounds");

        const size_t first_size = fixed_arr_n_elem - first_idx;
        if (index < first_size) {
            // Access within the first FixedArray
            return (*buffer.front())[first_idx + index];
        }

        const size_t adjusted_index = index - first_size;
        const size_t num_full_arrays = adjusted_index / fixed_arr_n_elem;
        const size_t index_in_array = adjusted_index % fixed_arr_n_elem;
        return (*buffer[num_full_arrays + 1])[index_in_array];
    }

    T &back() {
        assert(size_ > 0 && "Deque is empty");
        return (*buffer.back())[last_idx - 1];
    }

    T &front() {
        assert(size_ > 0 && "Deque is empty");
        return (*buffer.front())[first_idx];
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

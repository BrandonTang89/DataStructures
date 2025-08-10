#pragma once

#include <algorithm>
#include <cassert>
#include <memory>

template<typename T> requires std::default_initializable<T>
class CircularBuffer {
    static constexpr size_t min_cap{8}; // minimum capacity, ovoid unnecessary resizing
    std::unique_ptr<T[]> data;
    size_t begin{0};
    size_t end{0};
    size_t size_{0};
    size_t capacity_{0};

    void resize_up() {
        const size_t new_capacity = std::max(capacity_ * 2, min_cap);
        std::unique_ptr<T[]> new_data = std::make_unique<T[]>(new_capacity);

        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data[(begin + i) % capacity_]);
        }
        data = std::move(new_data);
        capacity_ = new_capacity;
        begin = 0;
        end = size_;
    }

    void resize_down() {
        const size_t new_capacity = std::max(capacity_ / 2, min_cap);
        std::unique_ptr<T[]> new_data = std::make_unique<T[]>(new_capacity);

        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data[(begin + i) % capacity_]);
        }
        data = std::move(new_data);
        capacity_ = new_capacity;
        begin = 0;
        end = size_;
    }

public:
    explicit CircularBuffer(const size_t _cap = min_cap) : capacity_(_cap) {
        assert(_cap >= 1 && "Capacity must be at least 1");
        data = std::make_unique<T[]>(_cap);
    }

    void push_back(T value) {
        if (size_ < capacity_) [[likely]] {
            data[end] = std::move(value);
            end = (end + 1) % capacity_;
        } else {
            // Buffer is full, need to resize
            resize_up();
            data[end] = std::move(value);
            end = end + 1;
        }
        ++size_;
    };

    void push_front(T value) {
        if (size_ < capacity_) [[likely]] {
            begin = (begin == 0 ? capacity_ - 1 : begin - 1);
            data[begin] = std::move(value);
        } else {
            // Buffer is full, need to resize
            resize_up();
            begin = capacity_ - 1;
            data[begin] = std::move(value);
        }
        ++size_;
    };

    T pop_back() {
        assert(size_ > 0 && "Cannot pop from an empty buffer");
        end = (end == 0 ? capacity_ - 1 : end - 1);
        T value = std::move(data[end]);
        --size_;
        if (size_ < capacity_ / 4 && capacity_ > min_cap) {
            resize_down();
        }
        return value;
    }

    T pop_front() {
        assert(size_ > 0 && "Cannot pop from an empty buffer");
        T value = std::move(data[begin]);
        begin = (begin + 1) % capacity_;
        --size_;
        if (size_ < capacity_ / 4 && capacity_ > min_cap) {
            resize_down();
        }
        return value;
    }

    T &operator[](const size_t index) {
        assert(index < size_ && "Index out of bounds");
        return data[(begin + index) % capacity_];
    }

    T &back() {
        assert(size_ > 0 && "Buffer is empty");
        return data[(end == 0 ? capacity_ - 1 : end - 1)];
    }

    T &front() {
        assert(size_ > 0 && "Buffer is empty");
        return data[begin];
    }

    [[nodiscard]] size_t size() const {
        return size_;
    }

    [[nodiscard]] size_t capacity() const {
        return capacity_;
    }

    [[nodiscard]] bool empty() const {
        return size_ == 0;
    }
};

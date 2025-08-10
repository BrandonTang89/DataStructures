#pragma once

#include <algorithm>
#include <cassert>
#include <memory>

template<typename T>
class CircularBuffer {
    static constexpr size_t min_cap{8}; // minimum capacity, ovoid unnecessary resizing
    struct AlignedDeleter {
        void operator()(std::byte *ptr) const {
            operator delete[](ptr, std::align_val_t{alignof(T)});
        }
    };
    std::unique_ptr<std::byte[], AlignedDeleter> data_storage;

    T *data{nullptr};
    size_t begin{0};
    size_t end{0};
    size_t size_{0};
    size_t capacity_{0};

    void resize(const size_t new_capacity) {
        std::unique_ptr<std::byte[], AlignedDeleter> new_storage{
            new(std::align_val_t{alignof(T)}) std::byte[new_capacity * sizeof(T)]
        };
        assert(new_storage && "Memory allocation failed");
        T *new_data = reinterpret_cast<T *>(new_storage.get());

        for (size_t i = 0; i < size_; ++i) {
            new(&new_data[i]) T(std::move(data[(begin + i) % capacity_]));
        }

        data_storage = std::move(new_storage);
        data = new_data;
        capacity_ = new_capacity;
        begin = 0;
        end = size_;
    }

    void resize_up() {
        const size_t new_capacity = std::max(capacity_ * 2, min_cap);
        resize(new_capacity);
    }

    void resize_down() {
        const size_t new_capacity = std::max(capacity_ / 2, min_cap);
        resize(new_capacity);
    }

    template<typename... Args>
    void emplace_back_impl(Args &&... args) {
        if (size_ < capacity_) [[likely]] {
            new(&data[end]) T(std::forward<Args>(args)...);
            end = (end + 1) % capacity_;
        } else {
            // Buffer is full, need to resize
            resize_up();
            new(&data[end]) T(std::forward<Args>(args)...);
            end = end + 1;
        }
        ++size_;
    }

    template<typename... Args>
    void emplace_front_impl(Args &&... args) {
        if (size_ < capacity_) [[likely]] {
            begin = begin == 0 ? capacity_ - 1 : begin - 1;
            new(&data[begin]) T(std::forward<Args>(args)...);
        } else {
            // Buffer is full, need to resize
            resize_up();
            begin = capacity_ - 1;
            new(&data[begin]) T(std::forward<Args>(args)...);
        }
        ++size_;
    }

public:
    explicit CircularBuffer(const size_t _cap = min_cap) : capacity_(_cap) {
        assert(_cap >= 1 && "Capacity must be at least 1");
        data_storage = std::unique_ptr<std::byte[], AlignedDeleter>(
            new(std::align_val_t{alignof(T)}) std::byte[_cap * sizeof(T)]
        );
        assert(data_storage && "Memory allocation failed");
        data = reinterpret_cast<T *>(data_storage.get());
        begin = 0;
        end = 0;
        size_ = 0;
    }

    ~CircularBuffer() {
        for (size_t i = 0; i < size_; ++i) {
            data[(begin + i) % capacity_].~T(); // Explicitly call destructor
        }
    }

    void push_back(const T &value) requires std::copy_constructible<T> {
        emplace_back_impl(value);
    }

    void push_back(T &&value) requires std::move_constructible<T> {
        emplace_back_impl(std::move(value));
    }

    template<typename... Args>
    void emplace_back(Args &&... args) requires std::constructible_from<T,
        Args...> {
        emplace_back_impl(std::forward<Args>(args)...);
    }

    void push_front(const T &value) requires std::copy_constructible<T> {
        emplace_front_impl(value);
    }

    void push_front(T &&value) requires std::move_constructible<T> {
        emplace_front_impl(std::move(value));
    }

    template<typename... Args>
    void emplace_front(Args &&... args) requires std::constructible_from<T,
        Args...> {
        emplace_front_impl(std::forward<Args>(args)...);
    }

    T pop_back() {
        assert(size_ > 0 && "Cannot pop from an empty buffer");
        end = end == 0 ? capacity_ - 1 : end - 1;
        T value = std::move(data[end]);
        data[end].~T();
        --size_;
        if (size_ < capacity_ / 4 && capacity_ > min_cap) {
            resize_down();
        }
        return value;
    }

    T pop_front() {
        assert(size_ > 0 && "Cannot pop from an empty buffer");
        T value = std::move(data[begin]);
        data[begin].~T(); // Explicitly call destructor
        begin = (begin + 1) % capacity_;
        --size_;
        if (size_ < capacity_ / 4 && capacity_ > min_cap) {
            resize_down();
        }
        return value;
    }

    template<typename Self>
    auto &&operator[](this Self &&self, const size_t index) {
        assert(index < self.size_ && "Index out of bounds");
        return self.data[(self.begin + index) % self.capacity_];
    }

    template<typename Self>
    auto &&back(this Self &&self) {
        assert(self.size_ > 0 && "Buffer is empty");
        return self.data[(self.end == 0 ? self.capacity_ - 1 : self.end - 1)];
    }

    template<typename Self>
    auto &&front(this Self &&self) {
        assert(self.size_ > 0 && "Buffer is empty");
        return self.data[self.begin];
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

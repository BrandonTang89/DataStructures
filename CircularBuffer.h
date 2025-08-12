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
    size_t begin_pos{0};
    size_t end_pos{0};
    size_t size_{0};
    size_t capacity_{0};

    void resize(const size_t new_capacity) {
        std::unique_ptr<std::byte[], AlignedDeleter> new_storage{
            new(std::align_val_t{alignof(T)}) std::byte[new_capacity * sizeof(T)]
        };
        assert(new_storage && "Memory allocation failed");
        T *new_data = reinterpret_cast<T *>(new_storage.get());

        for (size_t i = 0; i < size_; ++i) {
            new(&new_data[i]) T(std::move(data[(begin_pos + i) % capacity_]));
        }

        data_storage = std::move(new_storage);
        data = new_data;
        capacity_ = new_capacity;
        begin_pos = 0;
        end_pos = size_;
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
            new(&data[end_pos]) T(std::forward<Args>(args)...);
            end_pos = (end_pos + 1) % capacity_;
        } else {
            // Buffer is full, need to resize
            resize_up();
            new(&data[end_pos]) T(std::forward<Args>(args)...);
            end_pos = end_pos + 1;
        }
        ++size_;
    }

    template<typename... Args>
    void emplace_front_impl(Args &&... args) {
        if (size_ < capacity_) [[likely]] {
            begin_pos = begin_pos == 0 ? capacity_ - 1 : begin_pos - 1;
            new(&data[begin_pos]) T(std::forward<Args>(args)...);
        } else {
            // Buffer is full, need to resize
            resize_up();
            begin_pos = capacity_ - 1;
            new(&data[begin_pos]) T(std::forward<Args>(args)...);
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
        begin_pos = 0;
        end_pos = 0;
        size_ = 0;
    }

    // Rule of 5 Methods
    ~CircularBuffer() {
        for (size_t i = 0; i < size_; ++i) {
            data[(begin_pos + i) % capacity_].~T(); // Explicitly call destructor
        }
    }

    CircularBuffer &operator=(const CircularBuffer &) = delete;

    CircularBuffer(const CircularBuffer &) = delete;

    CircularBuffer &operator=(CircularBuffer &&other) noexcept {
        if (this != &other) {
            for (size_t i = 0; i < size_; ++i) {
                data[(begin_pos + i) % capacity_].~T(); // Explicitly call destructor
            }
            data_storage = std::move(other.data_storage);
            data = other.data;
            begin_pos = other.begin_pos;
            end_pos = other.end_pos;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data = nullptr; // Prevent double deletion
        }
        return *this;
    }

    CircularBuffer(CircularBuffer &&other) noexcept
        : data_storage(std::move(other.data_storage)), data(other.data),
          begin_pos(other.begin_pos), end_pos(other.end_pos), size_(other.size_),
          capacity_(other.capacity_) {
        other.data = nullptr; // Prevent double deletion
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
        end_pos = end_pos == 0 ? capacity_ - 1 : end_pos - 1;
        T value = std::move(data[end_pos]);
        data[end_pos].~T();
        --size_;
        if (size_ < capacity_ / 4 && capacity_ > min_cap) {
            resize_down();
        }
        return value;
    }

    T pop_front() {
        assert(size_ > 0 && "Cannot pop from an empty buffer");
        T value = std::move(data[begin_pos]);
        data[begin_pos].~T(); // Explicitly call destructor
        begin_pos = (begin_pos + 1) % capacity_;
        --size_;
        if (size_ < capacity_ / 4 && capacity_ > min_cap) {
            resize_down();
        }
        return value;
    }

    template<typename Self>
    auto &&operator[](this Self &&self, const size_t index) {
        assert(index < self.size_ && "Index out of bounds");
        return self.data[(self.begin_pos + index) % self.capacity_];
    }

    template<typename Self>
    auto &&back(this Self &&self) {
        assert(self.size_ > 0 && "Buffer is empty");
        return self.data[(self.end_pos == 0 ? self.capacity_ - 1 : self.end_pos - 1)];
    }

    template<typename Self>
    auto &&front(this Self &&self) {
        assert(self.size_ > 0 && "Buffer is empty");
        return self.data[self.begin_pos];
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

    struct Iterator {
        CircularBuffer *buffer;
        size_t index;

        Iterator(CircularBuffer *buf, const size_t idx) : buffer(buf), index(idx) {}

        template<typename Self>
        auto&& operator*(this Self&& self) {
            return self.buffer->data[(self.buffer->begin_pos + self.index) % self.buffer->capacity_];
        }

        template<typename Self>
        auto* operator->(this Self&& self) {
            return &self.buffer->data[(self.buffer->begin_pos + self.index) % self.buffer->capacity_];
        }

        Iterator& operator++() {
            ++index;
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++index;
            return temp;
        }

        Iterator& operator--() {
            --index;
            return *this;
        }

        Iterator operator--(int) {
            Iterator temp = *this;
            --index;
            return temp;
        }

        Iterator operator+(const size_t n) const {
            return Iterator(buffer, index + n);
        }

        Iterator operator-(const size_t n) const {
            return Iterator(buffer, index - n);
        }

        Iterator& operator+=(const size_t n) {
            index += n;
            return *this;
        }

        Iterator& operator-=(const size_t n) {
            index -= n;
            return *this;
        }

        ptrdiff_t operator-(const Iterator& other) const {
            return static_cast<ptrdiff_t>(index) - static_cast<ptrdiff_t>(other.index);
        }

        auto operator<=>(const Iterator& other) const {
            if (buffer != other.buffer) {
                return buffer <=> other.buffer;
            }
            return index <=> other.index;
        }

        bool operator==(const Iterator& other) const {
            return buffer == other.buffer && index == other.index;
        }
    };

    struct ConstIterator {
        const CircularBuffer *buffer;
        size_t index;

        ConstIterator(const CircularBuffer *buf, size_t idx) : buffer(buf), index(idx) {}

        template<typename Self>
        auto&& operator*(this Self&& self) {
            return self.buffer->data[(self.buffer->begin_pos + self.index) % self.buffer->capacity_];
        }

        template<typename Self>
        auto* operator->(this Self&& self) {
            return &self.buffer->data[(self.buffer->begin_pos + self.index) % self.buffer->capacity_];
        }

        ConstIterator& operator++() {
            ++index;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator temp = *this;
            ++index;
            return temp;
        }

        ConstIterator& operator--() {
            --index;
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator temp = *this;
            --index;
            return temp;
        }

        ConstIterator operator+(const size_t n) const {
            return ConstIterator(buffer, index + n);
        }

        ConstIterator operator-(const size_t n) const {
            return ConstIterator(buffer, index - n);
        }

        ConstIterator& operator+=(const size_t n) {
            index += n;
            return *this;
        }

        ConstIterator& operator-=(const size_t n) {
            index -= n;
            return *this;
        }

        ptrdiff_t operator-(const ConstIterator& other) const {
            return static_cast<ptrdiff_t>(index) - static_cast<ptrdiff_t>(other.index);
        }

        auto operator<=>(const ConstIterator& other) const {
            if (buffer != other.buffer) {
                return buffer <=> other.buffer;
            }
            return index <=> other.index;
        }

        bool operator==(const ConstIterator& other) const {
            return buffer == other.buffer && index == other.index;
        }
    };

    Iterator begin() {
        return Iterator(this, 0);
    }

    Iterator end() {
        return Iterator(this, size_);
    }

    ConstIterator begin() const {
        return ConstIterator(this, 0);
    }

    ConstIterator end() const {
        return ConstIterator(this, size_);
    }

    ConstIterator cbegin() const {
        return ConstIterator(this, 0);
    }

    ConstIterator cend() const {
        return ConstIterator(this, size_);
    }
};

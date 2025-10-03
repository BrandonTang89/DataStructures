#pragma once

#include <cassert>
#include <memory>

template<typename T>
class Vector {
    static constexpr size_t initial_capacity = 8;
    size_t capacity{initial_capacity};
    size_t size_{};

    struct AlignedDeleter {
        void operator()(std::byte *ptr) const {
            operator delete[](ptr, std::align_val_t{alignof(T)});
        }
    };

    std::unique_ptr<std::byte[], AlignedDeleter> data_storage;
    T *data{nullptr};

    void resize(const size_t new_capacity) {
        capacity = new_capacity;
        std::unique_ptr<std::byte[], AlignedDeleter> new_storage{
            new(static_cast<std::align_val_t>(alignof(T))) std::byte[capacity * sizeof(T)]
        };

        T *new_data = reinterpret_cast<T *>(new_storage.get());
        for (size_t i = 0; i < size_; ++i) {
            new(&new_data[i]) T(std::move(data[i]));
            data[i].~T();
        }

        data_storage = std::move(new_storage);
        data = reinterpret_cast<T *>(data_storage.get());
    }

    void resize_up() {
        if (size_ < capacity) {
            return;
        }
        resize(capacity * 2);
    }

    void resize_down() {
        if (size_ > capacity / 4 || capacity <= initial_capacity) {
            return;
        }
        resize(capacity / 2);
    }

public:
    Vector() {
        data_storage = std::unique_ptr<std::byte[], AlignedDeleter>(
            new(static_cast<std::align_val_t>(alignof(T))) std::byte[capacity * sizeof(T)]
        );
        data = reinterpret_cast<T *>(data_storage.get());
    }

    // Access Methods
    template<class Self>
    auto &&operator[](this Self &&self, const size_t index) {
        // preserves const
        return self.data[index];
    }

    [[nodiscard]] size_t size() const {
        return size_;
    }

    // Modifiers
    template<typename... Args>
    void emplace_back(Args &&... args) requires std::constructible_from<T, Args...> {
        resize_up();
        new(&data[size_]) T(std::forward<Args>(args)...);
        ++size_;
    }

    void push_back(const T &value) requires std::copy_constructible<T> {
        emplace_back(value);
    }

    void push_back(T &&value) requires std::move_constructible<T> {
        emplace_back(std::move(value));
    }

    T pop_back() {
        assert(size_ > 0 && "Cannot pop from an empty vector");
        --size_;
        T value = std::move(data[size_]);
        resize_down();
        return value;
    }

    // Rule of 5 Methods
    ~Vector() {
        for (size_t i = 0; i < size_; ++i) {
            data[i].~T(); // Explicitly call destructor for each element
        }
    }

    Vector(const Vector &other) requires std::copy_constructible<T> {
        resize(other.capacity);
        for (size_t i = 0; i < other.size_; ++i) {
            new(&data[i]) T(other.data[i]);
        }
        size_ = other.size_;
    }

    Vector &operator=(const Vector &other) requires std::copy_constructible<T> {
        if (this != &other) {
            for (size_t i = 0; i < size_; ++i) {
                data[i].~T();
            }
            resize(other.capacity);
            for (size_t i = 0; i < other.size_; ++i) {
                new(&data[i]) T(other.data[i]);
            }
            size_ = other.size_;
        }
        return *this;
    }

    Vector(Vector &&other) noexcept
        : capacity(other.capacity), size_(other.size_),
          data_storage(std::move(other.data_storage)), data(other.data) {
        other.capacity = initial_capacity;
        other.size_ = 0;
        other.data_storage = nullptr;
        other.data = nullptr;
    }

    Vector &operator=(Vector &&other) noexcept {
        if (this != &other) {
            for (size_t i = 0; i < size_; ++i) {
                data[i].~T();
            }
            capacity = other.capacity;
            size_ = other.size_;
            data_storage = std::move(other.data_storage); // current data_storage is freed
            data = other.data;

            other.capacity = initial_capacity;
            other.size_ = 0;
            other.data_storage = nullptr;
            other.data = nullptr;
        }
        return *this;
    }
};

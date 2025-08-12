#pragma once

#include <memory>
#include <vector>

// Class that caches allocations of T, calling the destructor and constructor appropriately
template<typename T>
class CachingAllocator {
    std::vector<T *> cache;
    size_t max_reserve{};

public:
    explicit CachingAllocator(const size_t initial_reserve = 0, const size_t max_reserve_ = 120) : max_reserve(
        max_reserve_) {
        cache.reserve(initial_reserve);
        for (size_t i = 0; i < initial_reserve; ++i) {
            cache.push_back(static_cast<T *>(operator new(sizeof(T), static_cast<std::align_val_t>(alignof(T)))));
        }
    }

    template<typename... Args> requires std::constructible_from<T, Args...>
    std::unique_ptr<T, CachingAllocator &> allocate(Args &&... args) {
        if (!cache.empty()) {
            auto ptr = cache.back();
            cache.pop_back();
            new(ptr) T(std::forward<Args>(args)...);
            return std::unique_ptr<T, CachingAllocator &>(ptr, *this);
        }
        return std::unique_ptr<T, CachingAllocator &>(new T(std::forward<Args>(args)...), *this);
    }

    void operator()(T *ptr) {
        // Call destructor on the object
        ptr->~T();
        if (cache.size() >= max_reserve) {
            delete ptr; // If cache is full, delete the pointer
            return;
        }
        // Store the pointer in the cache for future reuse
        cache.emplace_back(ptr);
    }

    // Rule of 5 Methods
    ~CachingAllocator() {
        for (auto ptr: cache) {
            delete ptr; // Clean up remaining cached pointers
        }
    }

    CachingAllocator &operator=(const CachingAllocator &) = delete;

    CachingAllocator(const CachingAllocator &) = delete;

    CachingAllocator(CachingAllocator &&other) noexcept = default;

    CachingAllocator &operator=(CachingAllocator &&other) noexcept {
        if (this != &other) {
            for (auto ptr: cache) {
                delete ptr; // Clean up existing cached pointers
            }
            cache = std::move(other.cache);
            max_reserve = other.max_reserve;
        }
        return *this;
    }
};

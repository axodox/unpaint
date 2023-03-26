#pragma once
#include "pch.h"

namespace Axodox::Collections
{
  template <typename T, size_t CAlignment = 16>
  class aligned_allocator
  {
  public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static const size_t alignment = CAlignment;

    template <typename U>
    struct rebind
    {
      typedef aligned_allocator<U, CAlignment> other;
    };

    aligned_allocator() { }
    aligned_allocator(const aligned_allocator&) { }
    template <typename U> aligned_allocator(const aligned_allocator<U, CAlignment>&) { }

    ~aligned_allocator() { }

    T* address(T& value) const
    {
      return &value;
    }

    const T* address(const T& value) const
    {
      return &value;
    }

    constexpr size_t max_size() const
    {
      return size_t(-1) / sizeof(T);
    }

    void construct(T* target, const T& value) const
    {
      auto location = static_cast<void*>(target);
      new (location) T(value);
    }

    void destroy(T* target) const
    {
      target->~T();
    }

    bool operator==(const aligned_allocator& other) const
    {
      return true;
    }

    bool operator!=(const aligned_allocator& other) const
    {
      return false;
    }

    T* allocate(size_t size) const
    {
      if (size == 0) return nullptr;
      if (size > max_size()) throw std::length_error("Cannot allocate the requested amount of memory.");

      size = (size + alignment - 1) & ~(alignment - 1);
      auto result = _aligned_malloc(size, alignment);

      return static_cast<T*>(result);
    }

    void deallocate(T* target, size_t /*n*/) const
    {
      _aligned_free(target);
    }

    template <typename U>
    T* allocate(const size_t n, const U* /* const hint */) const
    {
      return allocate(n);
    }

    aligned_allocator& operator=(const aligned_allocator&) = delete;
  };

  template<typename T, size_t CAlignment = 16>
  using aligned_vector = std::vector<T, Collections::aligned_allocator<T, CAlignment>>;
}
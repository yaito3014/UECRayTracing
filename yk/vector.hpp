#pragma once
#ifndef YK_RAYTRACING_VECTOR_H
#define YK_RAYTRACING_VECTOR_H

#include <cstddef>
#include <utility>

namespace yk {

// alternative to std::vector whose broken std::allocator implemented on MSVC
template <class T>
class vector {
 public:
  constexpr vector() noexcept = default;

  constexpr T* data() const { return ptr; }

  constexpr T* begin() const { return ptr; }
  constexpr T* end() const { return ptr + len; }

  constexpr T& operator[](std::size_t i) { return ptr[i]; }
  constexpr const T& operator[](std::size_t i) const { return ptr[i]; }

  constexpr void push_back(const T& x) {
    if (cap == len) extend();
    ptr[len++] = x;
  }

  constexpr void push_back(T&& x) {
    if (cap == len) extend();
    ptr[len++] = std::move(x);
  }

  constexpr void clear() noexcept {
    for (std::size_t i = len; i-- > 0;) ptr[i].~T();
    T* new_ptr = new (ptr) T[len];
    ptr = new_ptr;
    len = 0;
  }

  constexpr ~vector() noexcept { delete[] std::exchange(ptr, nullptr); }

 private:
  constexpr void extend() {
    if (ptr == nullptr || cap == 0)
      return ++cap, delete std::exchange(ptr, new T);
    cap *= 2;
    T* new_ptr = new T[cap]{};
    for (std::size_t i = 0; i < len; ++i) new_ptr[i] = std::move(ptr[i]);
    delete std::exchange(ptr, new_ptr);
  }
  T* ptr = nullptr;
  std::size_t len = 0;
  std::size_t cap = 0;
};

}  // namespace yk

#endif  // !YK_RAYTRACING_VECTOR_H

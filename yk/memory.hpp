#pragma once
#ifndef YK_RAYTRACING_MEMORY_H
#define YK_RAYTRACING_MEMORY_H

#include <compare>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace yk {

template <class T>
class unique_ptr {
 public:
  using element_type = T;
  using pointer = element_type*;

  constexpr unique_ptr() noexcept : ptr(nullptr) {}
  constexpr unique_ptr(std::nullptr_t) noexcept : unique_ptr() {}

  explicit constexpr unique_ptr(pointer p) noexcept : ptr(p) {}

  constexpr unique_ptr(unique_ptr&& other) noexcept
      : ptr(std::exchange(other.ptr, nullptr)) {}

  template <class U>
  constexpr unique_ptr(unique_ptr<U>&& other)
      : ptr(std::exchange(other.ptr, nullptr)) {}

  constexpr ~unique_ptr() noexcept { delete ptr; }

  constexpr unique_ptr& operator=(unique_ptr&& other) {
    ptr = std::exchange(other.ptr, nullptr);
    return *this;
  }

  template <class U>
  constexpr unique_ptr& operator=(unique_ptr<U>&& other) {
    ptr = std::exchange(other.ptr, nullptr);
    return *this;
  }

  constexpr std::add_lvalue_reference_t<T> operator*() const { return *get(); }
  constexpr pointer operator->() const noexcept { return get(); }
  constexpr pointer get() const noexcept { return ptr; }

  constexpr explicit operator bool() const noexcept { return get() != nullptr; }

  constexpr pointer release() noexcept {
    return delete ptr, std::exchange(ptr, nullptr);
  }

  void swap(unique_ptr& other) { std::swap(ptr, other.ptr); }

  friend constexpr auto operator<=>(const unique_ptr& x, const unique_ptr& y) {
    return x.get() <=> y.get();
  }

  template <class>
  friend class unique_ptr;

 private:
  pointer ptr;
};

template <class T, class... Args>
constexpr unique_ptr<T> make_unique(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
constexpr void swap(unique_ptr<T>& x, unique_ptr<T>& y) {
  x.swap(y);
}

}  // namespace yk

#endif  // !YK_RAYTRACING_MEMORY_H

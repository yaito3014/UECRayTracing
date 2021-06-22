#pragma once

#ifndef YK_RAYTRACING_COLOR_H
#define YK_RAYTRACING_COLOR_H

#include <algorithm>
#include <cstdint>

#include "concepts.hpp"

namespace yk {

template <concepts::arithmetic T>
struct alignas(T) color3 {
  using value_type = T;
  T r, g, b;

  template <concepts::arithmetic U>
  constexpr color3<U> to() const noexcept {
    return {
        .r = static_cast<U>(r),
        .g = static_cast<U>(g),
        .b = static_cast<U>(b),
    };
  }

  constexpr color3 &clamp(T min, T max) noexcept {
    r = std::clamp(r, min, max);
    g = std::clamp(g, min, max);
    b = std::clamp(b, min, max);
    return *this;
  }

  constexpr color3 clamped(T min, T max) const noexcept {
    return {
        .r = std::clamp(r, min, max),
        .g = std::clamp(g, min, max),
        .b = std::clamp(b, min, max),
    };
  }

  constexpr bool operator==(const color3 &) const noexcept = default;

  constexpr color3 operator-() const noexcept { return {-r, -g, -b}; }

  constexpr color3 &operator+=(const color3 &rhs) noexcept {
    r += rhs.r;
    g += rhs.g;
    b += rhs.b;
    return *this;
  }

  constexpr color3 &operator-=(const color3 &rhs) noexcept {
    r -= rhs.r;
    g -= rhs.g;
    b -= rhs.b;
    return *this;
  }

  constexpr color3 &operator*=(const color3 &rhs) noexcept {
    r *= rhs.r;
    g *= rhs.g;
    b *= rhs.b;
    return *this;
  }

  template <concepts::arithmetic U>
  constexpr color3 &operator*=(U rhs) noexcept {
    r *= rhs;
    g *= rhs;
    b *= rhs;
    return *this;
  }

  template <concepts::arithmetic U>
  constexpr color3 &operator/=(U rhs) noexcept {
    r /= rhs;
    g /= rhs;
    b /= rhs;
    return *this;
  }
};

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator+(const color3<T> &lhs, const color3<U> &rhs) noexcept {
  return color3{lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator-(const color3<T> &lhs, const color3<U> &rhs) noexcept {
  return color3{lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator*(const color3<T> &lhs, const color3<U> &rhs) noexcept {
  return color3{lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator*(const color3<T> &color, U scalar) noexcept {
  return color3{color.r * scalar, color.g * scalar, color.b * scalar};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator*(T scalar, color3<U> color) noexcept {
  return color3{color.r * scalar, color.g * scalar, color.b * scalar};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator/(color3<T> color, U scalar) noexcept {
  return color3{color.r / scalar, color.g / scalar, color.b / scalar};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator/(T scalar, color3<U> color) noexcept {
  return color3{color.r / scalar, color.g / scalar, color.b / scalar};
}

using color3b = color3<uint8_t>;
using color3d = color3<double>;

}  // namespace yk

#endif  // !YK_RAYTRACING_COLOR_H

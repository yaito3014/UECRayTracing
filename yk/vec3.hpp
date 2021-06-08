#pragma once

#ifndef YK_RAYTRACING_VEC3_H
#define YK_RAYTRACING_VEC3_H

#include <type_traits>
#include <random>

#include "concepts.hpp"
#include "math.hpp"
#include "random.hpp"

namespace yk {

template <concepts::arithmetic T>
struct vec3 {
  T x, y, z;

  template <concepts::arithmetic U>
  constexpr vec3<U> to() const {
    return {
        .x = static_cast<U>(x),
        .y = static_cast<U>(y),
        .z = static_cast<U>(z),
    };
  }

  constexpr bool operator==(const vec3 &) const = default;

  constexpr vec3 operator-() const { return {-x, -y, -z}; }

  constexpr vec3 &operator+=(const vec3 &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  constexpr vec3 &operator-=(const vec3 &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  constexpr vec3 &operator*=(concepts::arithmetic auto rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  template <concepts::arithmetic U>
  constexpr vec3 &operator/=(U rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  template <concepts::arithmetic U>
  constexpr auto dot(const vec3<U> &rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  template <concepts::arithmetic U>
  constexpr auto cross(const vec3<U> &rhs) const {
    return vec3{.x = y * rhs.z - z * rhs.y,
                .y = z * rhs.x - x * rhs.z,
                .z = x * rhs.y - y * rhs.x};
  }

  constexpr auto length() const { return math::sqrt(x * x + y * y + z * z); }

  constexpr T length_squared() const { return x * x + y * y + z * z; }

  constexpr vec3 &normalize() { return *this /= length(); }
  constexpr vec3 normalized() const { return *this / length(); }

  template <std::uniform_random_bit_generator Gen>
  constexpr static vec3 random(Gen& gen, T min, T max) {
    uniform_real_distribution<T> dist(min, max);
    return {
      .x = dist(gen),
      .y = dist(gen),
      .z = dist(gen),
    };
  }
};

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator+(const vec3<T> &lhs, const vec3<U> &rhs) {
  return vec3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator-(const vec3<T> &lhs, const vec3<U> &rhs) {
  return vec3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator*(const vec3<T> &vec, U scalar) {
  return vec3{vec.x * scalar, vec.y * scalar, vec.z * scalar};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator*(T scalar, const vec3<U> &vec) {
  return vec * scalar;
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator/(const vec3<T> &vec, U scalar) {
  return vec3{vec.x / scalar, vec.y / scalar, vec.z / scalar};
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto operator/(T scalar, const vec3<U> &vec) {
  return vec / scalar;
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto dot(const vec3<T> &lhs, const vec3<U> &rhs) {
  return lhs.dot(rhs);
}

template <concepts::arithmetic T, concepts::arithmetic U>
constexpr auto cross(const vec3<T> &lhs, const vec3<U> &rhs) {
  return lhs.cross(rhs);
}

using vec3d = vec3<double>;

template <class T>
using pos3 = vec3<T>;

using pos3d = pos3<double>;

}  // namespace yk

#endif  // !YK_RAYTRACING_VEC3_H

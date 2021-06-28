#pragma once

#ifndef YK_RAYTRACING_MATH_H
#define YK_RAYTRACING_MATH_H

#include <cmath>
#include <concepts>

#include "concepts.hpp"

namespace yk::math {

template <concepts::arithmetic T>
constexpr auto sqrt(T s) {
  T x = s / 2.0;
  T prev = 0.0;
  while (x != prev) {
    prev = x;
    x = (x + s / x) / 2.0;
  }
  return x;
};

template <concepts::arithmetic T>
constexpr T abs(T a) noexcept {
  return a > 0 ? a : -a;
}

template <concepts::arithmetic T, std::integral U>
constexpr T pow(T a, U b) noexcept {
  T res = 1;
  while (b) {
    if (b % 2) res *= a;
    if (b /= 2) a *= a;
  }
  return res;
}

using std::cos;
using std::sin;
using std::tan;

}  // namespace yk::math

#endif  // !YK_RAYTRACING_MATH_H

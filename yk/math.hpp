#pragma once

#ifndef YK_RAYTRACING_MATH_H
#define YK_RAYTRACING_MATH_H

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

}  // namespace yk::math

#endif  // !YK_RAYTRACING_MATH_H

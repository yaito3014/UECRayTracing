#pragma once

#ifndef YK_RAYTRACING_RANDOM_H
#define YK_RAYTRACING_RANDOM_H

#include <cstdint>
#include <limits>
#include <random>

#include "concepts.hpp"

namespace yk {

struct xor128 {
  uint32_t x = 123456789;
  uint32_t y = 362436069;
  uint32_t z = 521288629;
  uint32_t w = 88675123;

  constexpr xor128(uint32_t seed) : w(seed) {}

  constexpr uint32_t operator()() {
    uint32_t t = x ^ (x << 11);
    x = y;
    y = z;
    z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
  }
};

template <concepts::arithmetic T>
struct uniform_real_distribution {
  T min;
  T max;
  constexpr uniform_real_distribution(T min_, T max_) : min(min_), max(max_) {}

  template <class Engine>
  constexpr T operator()(Engine& e) {
    using From = decltype(e());
    auto r = e();
    return (r - std::numeric_limits<From>::min()) *
        (std::numeric_limits<T>::max() - std::numeric_limits<T>::min()) /
        (std::numeric_limits<From>::max() - std::numeric_limits<From>::min());
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_RANDOM_H

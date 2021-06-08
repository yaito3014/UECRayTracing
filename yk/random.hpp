#pragma once

#ifndef YK_RAYTRACING_RANDOM_H
#define YK_RAYTRACING_RANDOM_H

#include <cstdint>
#include <limits>
#include <random>
#include <utility>

#include "concepts.hpp"

namespace yk {

struct xor128 {
  uint32_t x = 123456789;
  uint32_t y = 362436069;
  uint32_t z = 521288629;
  uint32_t w = 88675123;

  using result_type = uint32_t;
  constexpr static result_type min() {
    return std::numeric_limits<uint32_t>::min();
  }
  constexpr static result_type max() {
    return std::numeric_limits<uint32_t>::max();
  }

  constexpr xor128(uint32_t seed) : w(88675123 ^ seed) {}

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

  template <std::uniform_random_bit_generator G>
  constexpr T operator()(G& gen) {
    auto r = gen();
    return (r - std::numeric_limits<typename G::result_type>::min()) *
           (max - min) / (G::max() - G::min());
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_RANDOM_H

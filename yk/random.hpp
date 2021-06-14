#pragma once

#ifndef YK_RAYTRACING_RANDOM_H
#define YK_RAYTRACING_RANDOM_H

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
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

template <typename UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a,
          size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c,
          size_t l, UIntType f>
class mersenne_twister_engine {
 public:
  typedef UIntType result_type;

  static constexpr size_t word_size = w;
  static constexpr size_t state_size = n;
  static constexpr size_t shift_size = m;
  static constexpr size_t mask_bits = r;
  static constexpr result_type xor_mask = a;
  static constexpr size_t tempering_u = u;
  static constexpr result_type tempering_d = d;
  static constexpr size_t tempering_s = s;
  static constexpr result_type tempering_b = b;
  static constexpr size_t tempering_t = t;
  static constexpr result_type tempering_c = c;
  static constexpr size_t tempering_l = l;
  static constexpr result_type initialization_multiplier = f;
  static constexpr result_type default_seed = 5489u;

  constexpr mersenne_twister_engine() : mersenne_twister_engine(default_seed) {}

  constexpr explicit mersenne_twister_engine(result_type sd) { seed(sd); }

  constexpr void seed(result_type sd = default_seed) {
    M_x[0] =
        (UIntType(1) << w) ? UIntType(sd) % (UIntType(1) << w) : UIntType(sd);
    for (size_t i = 1; i < state_size; ++i) {
      UIntType x = M_x[i - 1];
      x ^= x >> (w - 2);
      x *= f;
      x += i % n;
      M_x[i] =
          (UIntType(1) << w) ? UIntType(x) % (UIntType(1) << w) : UIntType(x);
    }
    M_p = state_size;
  }

  static constexpr result_type min() { return 0; }

  static constexpr result_type max() { return (UIntType(1) << w) - 1; }

  constexpr void discard(unsigned long long z) {
    while (z > state_size - M_p) {
      z -= state_size - M_p;
      M_gen_rand();
    }
    M_p += z;
  }

  constexpr result_type operator()() {
    if (M_p >= state_size) M_gen_rand();

    result_type z = M_x[M_p++];
    z ^= (z >> u) & d;
    z ^= (z << s) & b;
    z ^= (z << t) & c;
    z ^= (z >> l);

    return z;
  }

  constexpr friend bool operator==(const mersenne_twister_engine& lhs,
                                   const mersenne_twister_engine& rhs) {
    return (std::equal(lhs.M_x, lhs.M_x + state_size, rhs.M_x) &&
            lhs.M_p == rhs.M_p);
  }

 private:
  constexpr void M_gen_rand() {
    const UIntType upper_mask = (~UIntType()) << r;
    const UIntType lower_mask = ~upper_mask;

    for (size_t k = 0; k < (n - m); ++k) {
      UIntType y = ((M_x[k] & upper_mask) | (M_x[k + 1] & lower_mask));
      M_x[k] = (M_x[k + m] ^ (y >> 1) ^ ((y & 0x01) ? a : 0));
    }

    for (size_t k = (n - m); k < (n - 1); ++k) {
      UIntType y = ((M_x[k] & upper_mask) | (M_x[k + 1] & lower_mask));
      M_x[k] = (M_x[k + (m - n)] ^ (y >> 1) ^ ((y & 0x01) ? a : 0));
    }

    UIntType y = ((M_x[n - 1] & upper_mask) | (M_x[0] & lower_mask));
    M_x[n - 1] = (M_x[m - 1] ^ (y >> 1) ^ ((y & 0x01) ? a : 0));
    M_p = 0;
  }

  UIntType M_x[state_size];
  size_t M_p;
};

template <typename UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a,
          size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c,
          size_t l, UIntType f>
inline bool operator!=(
    const mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l,
                                  f>& lhs,
    const mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l,
                                  f>& rhs) {
  return !(lhs == rhs);
}

typedef mersenne_twister_engine<uint_fast32_t, 32, 624, 397, 31, 0x9908b0dfUL,
                                11, 0xffffffffUL, 7, 0x9d2c5680UL, 15,
                                0xefc60000UL, 18, 1812433253UL>
    mt19937;

typedef mersenne_twister_engine<
    uint_fast64_t, 64, 312, 156, 31, 0xb5026f5aa96619e9ULL, 29,
    0x5555555555555555ULL, 17, 0x71d67fffeda60000ULL, 37, 0xfff7eee000000000ULL,
    43, 6364136223846793005ULL>
    mt19937_64;

namespace detail {

template <typename RealType, size_t bits, typename UniformRandomNumberGenerator>
constexpr RealType generate_canonical(UniformRandomNumberGenerator& urng) {
  static_assert(std::is_floating_point<RealType>::value,
                "template argument must be a floating point type");

  const size_t b = std::min(
      static_cast<size_t>(std::numeric_limits<RealType>::digits), bits);
  const long double r = static_cast<long double>(urng.max()) -
                        static_cast<long double>(urng.min()) + 1.0L;
  const size_t log2r = std::bit_width(static_cast<size_t>(r));
  const size_t m = std::max<size_t>(1UL, (b + log2r - 1UL) / log2r);
  RealType ret;
  RealType sum = RealType(0);
  RealType tmp = RealType(1);
  for (size_t k = m; k != 0; --k) {
    sum += RealType(urng() - urng.min()) * tmp;
    tmp *= r;
  }
  ret = sum / tmp;
  if (ret >= RealType(1)) [[unlikely]]
    ret = RealType(1) - std::numeric_limits<RealType>::epsilon() / RealType(2);
  return ret;
}

template <typename Engine, typename DInputType>
struct Adaptor {
  static_assert(std::is_floating_point<DInputType>::value,
                "template argument must be a floating point type");

 public:
  constexpr Adaptor(Engine& g) : M_g(g) {}

  constexpr DInputType min() const { return DInputType(0); }

  constexpr DInputType max() const { return DInputType(1); }

  constexpr DInputType operator()() {
    return generate_canonical<DInputType,
                              std::numeric_limits<DInputType>::digits, Engine>(
        M_g);
  }

 private:
  Engine& M_g;
};

}  // namespace detail

template <typename RealType = double>
class uniform_real_distribution {
  static_assert(std::is_floating_point<RealType>::value,
                "result_type must be a floating point type");

 public:
  typedef RealType result_type;

  struct param_type {
    typedef uniform_real_distribution<RealType> distribution_type;

    constexpr param_type() : param_type(0) {}

    constexpr explicit param_type(RealType a, RealType b = RealType(1))
        : M_a(a), M_b(b) {}

    constexpr result_type a() const { return M_a; }

    constexpr result_type b() const { return M_b; }

    constexpr friend bool operator==(const param_type& p1,
                                     const param_type& p2) {
      return p1.M_a == p2.M_a && p1.M_b == p2.M_b;
    }

    constexpr friend bool operator!=(const param_type& p1,
                                     const param_type& p2) {
      return !(p1 == p2);
    }

   private:
    RealType M_a;
    RealType M_b;
  };

 public:
  constexpr uniform_real_distribution() : uniform_real_distribution(0.0) {}

  constexpr explicit uniform_real_distribution(RealType a,
                                               RealType b = RealType(1))
      : M_param(a, b) {}

  constexpr explicit uniform_real_distribution(const param_type& p)
      : M_param(p) {}

  constexpr void reset() {}

  constexpr result_type a() const { return M_param.a(); }

  constexpr result_type b() const { return M_param.b(); }

  constexpr param_type param() const { return M_param; }

  constexpr void param(const param_type& param) { M_param = param; }

  constexpr result_type min() const { return this->a(); }

  constexpr result_type max() const { return this->b(); }

  template <typename UniformRandomNumberGenerator>
  constexpr result_type operator()(UniformRandomNumberGenerator& urng) {
    return this->operator()(urng, M_param);
  }

  template <typename UniformRandomNumberGenerator>
  constexpr result_type operator()(UniformRandomNumberGenerator& urng,
                                   const param_type& p) {
    detail::Adaptor<UniformRandomNumberGenerator, result_type> aurng(urng);
    return (aurng() * (p.b() - p.a())) + p.a();
  }

  constexpr friend bool operator==(const uniform_real_distribution& d1,
                                   const uniform_real_distribution& d2) {
    return d1.M_param == d2.M_param;
  }

 private:
  param_type M_param;
};

template <typename IntType>
constexpr inline bool operator!=(
    const std::uniform_real_distribution<IntType>& d1,
    const std::uniform_real_distribution<IntType>& d2) {
  return !(d1 == d2);
}

}  // namespace yk

#endif  // !YK_RAYTRACING_RANDOM_H

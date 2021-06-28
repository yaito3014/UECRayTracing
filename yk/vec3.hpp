#pragma once

#ifndef YK_RAYTRACING_VEC3_H
#define YK_RAYTRACING_VEC3_H

#include <random>
#include <type_traits>

#include "concepts.hpp"
#include "math.hpp"
#include "random.hpp"

namespace yk {

struct tag_base {};

namespace concepts {

template <class Tag>
concept tag = std::derived_from<Tag, tag_base>;

}  // namespace concepts

struct default_tag : tag_base {};
struct world_tag : default_tag {};

template <concepts::arithmetic T, concepts::tag Tag = default_tag>
struct vec3 {
  T x, y, z;

  template <concepts::arithmetic U>
  constexpr vec3<U, Tag> to() const noexcept {
    return {
        .x = static_cast<U>(x),
        .y = static_cast<U>(y),
        .z = static_cast<U>(z),
    };
  }

  // clang-format off
  template <concepts::tag ToTag>
    requires (not std::same_as<Tag, default_tag>)
              && std::same_as<ToTag, default_tag>
  constexpr vec3<T, default_tag> to() const noexcept{
    return {x, y, z};
  }

  template <concepts::tag ToTag>
    requires (not std::same_as<Tag, default_tag>)
              && std::derived_from<ToTag, Tag>
  constexpr vec3<T, ToTag> to(vec3 origin) const noexcept {
    return {
        .x = origin.x + x,
        .y = origin.y + y,
        .z = origin.z + z,
    };
  }

  template <concepts::tag ToTag>
    requires (not std::same_as<Tag, default_tag>)
              && std::derived_from<Tag, ToTag>
  constexpr vec3<T, ToTag> to(vec3<T, ToTag> origin) const noexcept{
    return {
        .x = x - origin.x,
        .y = y - origin.y,
        .z = z - origin.z,
    };
  }
  // clang-format on

  template <concepts::arithmetic U, concepts::tag ToTag>
  constexpr vec3<U, ToTag> to(vec3 origin) const noexcept {
    return to<ToTag>(origin).template to<U>();
  }

  constexpr bool near_zero() const noexcept {
    constexpr auto s = 1e-8;
    return (math::abs(x) < s) && (math::abs(y) < s) && (math::abs(x) < s);
  }

  constexpr bool operator==(const vec3 &) const noexcept = default;

  constexpr vec3 operator-() const noexcept { return {-x, -y, -z}; }

  constexpr vec3 &operator+=(const vec3 &rhs) noexcept {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  constexpr vec3 &operator-=(const vec3 &rhs) noexcept {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  constexpr vec3 &operator*=(concepts::arithmetic auto rhs) noexcept {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  template <concepts::arithmetic U>
  constexpr vec3 &operator/=(U rhs) noexcept {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  template <concepts::arithmetic U>
  constexpr auto dot(const vec3<U, Tag> &rhs) const noexcept {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  template <concepts::arithmetic U>
  constexpr auto cross(const vec3<U, Tag> &rhs) const noexcept {
    return vec3{.x = y * rhs.z - z * rhs.y,
                .y = z * rhs.x - x * rhs.z,
                .z = x * rhs.y - y * rhs.x};
  }

  constexpr auto length() const noexcept {
    return math::sqrt(x * x + y * y + z * z);
  }

  constexpr T length_squared() const noexcept { return x * x + y * y + z * z; }

  constexpr vec3 &normalize() noexcept { return *this /= length(); }
  constexpr vec3 normalized() const noexcept { return *this / length(); }

  template <std::uniform_random_bit_generator Gen>
  constexpr static vec3 random(Gen &gen, T min, T max) noexcept {
    uniform_real_distribution<T> dist(min, max);
    return {
        .x = dist(gen),
        .y = dist(gen),
        .z = dist(gen),
    };
  }
};

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto operator+(const vec3<T, Tag> &lhs,
                         const vec3<U, Tag> &rhs) noexcept {
  return vec3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto operator-(const vec3<T, Tag> &lhs,
                         const vec3<U, Tag> &rhs) noexcept {
  return vec3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
  requires(not std::same_as<Tag, default_tag>)
constexpr auto operator+(const vec3<T, Tag> &lhs,
                         const vec3<U, default_tag> &rhs) noexcept {
  return vec3<T, Tag>{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
  requires(not std::same_as<Tag, default_tag>)
constexpr auto operator-(const vec3<T, Tag> &lhs,
                         const vec3<U, default_tag> &rhs) noexcept {
  return vec3<T, Tag>{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto operator*(const vec3<T, Tag> &vec, U scalar) noexcept {
  return vec3{vec.x * scalar, vec.y * scalar, vec.z * scalar};
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto operator*(T scalar, const vec3<U, Tag> &vec) noexcept {
  return vec * scalar;
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto operator/(const vec3<T, Tag> &vec, U scalar) noexcept {
  return vec3{vec.x / scalar, vec.y / scalar, vec.z / scalar};
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto operator/(T scalar, const vec3<U, Tag> &vec) noexcept {
  return vec / scalar;
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto dot(const vec3<T, Tag> &lhs, const vec3<U, Tag> &rhs) noexcept {
  return lhs.dot(rhs);
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto cross(const vec3<T, Tag> &lhs,
                     const vec3<U, Tag> &rhs) noexcept {
  return lhs.cross(rhs);
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag>
constexpr auto reflect(const vec3<T, Tag> &vec,
                       const vec3<U, Tag> &norm) noexcept {
  return vec - 2 * dot(vec, norm) * norm;
}

template <concepts::arithmetic T, concepts::arithmetic U, concepts::tag Tag,
          concepts::arithmetic S>
constexpr auto refract(const vec3<T, Tag> &uv, const vec3<U, Tag> &norm,
                       S etai_over_etat) noexcept {
  auto cos_theta = std::min(dot(-uv, norm), 1.);
  auto r_out_perp = etai_over_etat * (uv + cos_theta * norm);
  auto r_out_parallel =
      -math::sqrt(math::abs(1 - r_out_perp.length_squared())) * norm;
  return r_out_perp + r_out_parallel;
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_in_unit_sphere(Gen &gen) noexcept {
  return vec3<T>::random(gen, -1, 1).normalize() *
         uniform_real_distribution<T>(0.01, 0.99)(gen);
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_unit_vector(Gen &gen) noexcept {
  return vec3<T>::random(gen, -1, 1).normalize();
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_in_hemisphere(const vec3<T> &normal,
                                       Gen &gen) noexcept {
  vec3<T> in_unit_sphere = random_unit_vector<T>(gen);
  return dot(in_unit_sphere, normal) > 0.0 ? in_unit_sphere : -in_unit_sphere;
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_in_unit_disk(Gen &gen) noexcept {
  uniform_real_distribution<T> dist(-1, 1);
  auto p = vec3<T>(dist(gen), dist(gen), 0);
  if (p.length_squared() >= 1) {
    uniform_real_distribution<T> dist(0, 1);
    p = p.normalized() * dist(gen);
  }
  return p;
}

using vec3d = vec3<double>;

template <class T, concepts::tag Tag>
using pos3 = vec3<T, Tag>;

}  // namespace yk

#endif  // !YK_RAYTRACING_VEC3_H

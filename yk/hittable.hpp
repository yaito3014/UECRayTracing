#pragma once

#ifndef YK_RAYTRACING_HITTABLE_H
#define YK_RAYTRACING_HITTABLE_H

#include <concepts>
#include <type_traits>

#include "concepts.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T>
struct hit_record {
  pos3<T> p;
  vec3<T> normal;
  T t;
  bool front_face;
  constexpr void set_face_normal(const ray<T>& r,
                                 const vec3<T>& outward_normal) {
    front_face = dot(r.direction, outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

template <concepts::arithmetic T, class Derived>
struct hittable_interface {
  constexpr bool hit(const ray<T>& r, T t_min, T t_max, hit_record<T>& rec) const {
    return static_cast<const Derived*>(this)->hit_impl(r, t_min, t_max, rec);
  }
};

namespace concepts {

// clang-format off
template <class H, class T>
concept hittable =
    arithmetic<T> && std::is_base_of_v<hittable_interface<T, H>, H>;
// clang-format on

}  // namespace concepts

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_H

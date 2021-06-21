#pragma once

#ifndef YK_RAYTRACING_HITTABLE_H
#define YK_RAYTRACING_HITTABLE_H

#include <concepts>
#include <type_traits>
#include <optional>

#include "concepts.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T>
struct hit_record {
  pos3<T, world_tag> p;
  vec3<T> normal;
  T t;
  size_t id;
  bool front_face;
  constexpr void set_face_normal(const ray<T>& r,
                                 const vec3<T>& outward_normal) {
    front_face = dot(r.direction, outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

template <concepts::arithmetic T, class Derived>
struct hittable_interface {
  constexpr std::optional<hit_record<T>> hit(const ray<T>& r, T t_min, T t_max) const {
    return static_cast<const Derived*>(this)->hit_impl(r, t_min, t_max);
  }

  // returns optioanl of pair of { color3<U> attenuation, ray<T> scattered }
  template <concepts::arithmetic U, std::uniform_random_bit_generator Gen>
  constexpr std::optional<std::pair<color3<U>, ray<T>>> scatter(const ray<T>& r, const hit_record<T>& rec, Gen& gen) const {
    return static_cast<const Derived*>(this)->template scatter_impl<U>(r, rec, gen);
  }
};

namespace concepts {

template <class H, class T>
concept hittable =
    arithmetic<T> && std::is_base_of_v<hittable_interface<T, H>, H>;

}  // namespace concepts

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_H

#pragma once

#ifndef YK_RAYTRACING_HITTABLE_H
#define YK_RAYTRACING_HITTABLE_H

#include <concepts>
#include <type_traits>

#include "aabb.hpp"
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
                                 const vec3<T>& outward_normal) noexcept {
    front_face = dot(r.direction, outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

template <concepts::arithmetic T, class Derived>
struct hittable_interface {
  constexpr bool hit(const ray<T>& r, T t_min, T t_max,
                     hit_record<T>& rec) const noexcept {
    return static_cast<const Derived*>(this)->hit_impl(r, t_min, t_max, rec);
  }

  constexpr bool bouding_box(T time0, T time1,
                             aabb<T>& output_box) const noexcept {
    return static_cast<const Derived*>(this)->bouding_box_impl(time0, time1,
                                                               output_box);
  }

  template <concepts::arithmetic U, std::uniform_random_bit_generator Gen>
  constexpr bool scatter(const ray<T>& r, const hit_record<T>& rec,
                         color3<U>& attenuation, ray<T>& scattered,
                         Gen& gen) const noexcept {
    return static_cast<const Derived*>(this)->template scatter_impl<U>(
        r, rec, attenuation, scattered, gen);
  }
};

namespace concepts {

template <class H, class T>
concept hittable =
    arithmetic<T> && std::is_base_of_v<hittable_interface<T, H>, H>;

}  // namespace concepts

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_H

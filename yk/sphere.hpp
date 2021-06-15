#pragma once
#ifndef YK_RAYTRACING_SPHERE_H
#define YK_RAYTRACING_SPHERE_H

#include <optional>

#include "concepts.hpp"
#include "hittable.hpp"
#include "raytracer.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T>
struct sphere : public hittable_interface<T, sphere<T>> {
  pos3<T, world_tag> center;
  T radius;

  constexpr sphere(pos3<T, world_tag> center, T radius)
      : center(center), radius(radius) {}

  constexpr std::optional<hit_record<T>> hit_impl(const ray<T>& r, T t_min, T t_max) const {
    vec3<T> oc = r.origin - center;
    auto a = r.direction.length_squared();
    auto half_b = dot(oc, r.direction);
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return std::nullopt;
    auto sqrtd = math::sqrt(discriminant);

    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
      root = (-half_b + sqrtd) / a;
      if (root < t_min || t_max < root) return std::nullopt;
    }

    hit_record<T> rec;
    rec.t = root;
    rec.p = r.at(rec.t);
    auto outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);

    return rec;
  }

  template <concepts::arithmetic U>
  constexpr color3<U> get_color_impl(const raytracer<T, U>&tracer, const ray<T>& r, const hit_record<T>& rec, T mul = 1, T add = 0) const {

  }

};

}  // namespace yk

#endif  // !YK_RAYTRACING_SPHERE_H

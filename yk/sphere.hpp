#pragma once
#ifndef YK_RAYTRACING_SPHERE_H
#define YK_RAYTRACING_SPHERE_H

#include "concepts.hpp"
#include "hittable.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T>
struct sphere : public hittable_interface<T, sphere<T>> {
  pos3<T> center;
  T radius;

  constexpr sphere(pos3<T> center, T radius) : center(center), radius(radius) {}

  constexpr bool hit_impl(const ray<T>& r, T t_min, T t_max,
                             hit_record<T>& rec) const {
    vec3<T> oc = r.origin - center;
    auto a = r.direction.length_squared();
    auto half_b = dot(oc, r.direction);
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = math::sqrt(discriminant);

    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
      root = (-half_b + sqrtd) / a;
      if (root < t_min || t_max < root) return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    auto outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);

    return true;
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_SPHERE_H

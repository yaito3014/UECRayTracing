#pragma once
#ifndef YK_RAYTRACING_SPHERE_H
#define YK_RAYTRACING_SPHERE_H

#include <optional>
#include <random>

#include "concepts.hpp"
#include "hittable.hpp"
#include "material.hpp"
#include "raytracer.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T, concepts::material M>
struct sphere : public hittable_interface<T, sphere<T, M>> {
  pos3<T, world_tag> center;
  T radius;
  M material;

  constexpr sphere(pos3<T, world_tag> center, T radius, M material)
      : center(center), radius(radius), material(material) {}

  constexpr std::optional<hit_record<T>> hit_impl(const ray<T>& r, T t_min,
                                                  T t_max) const {
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

    hit_record<T> rec = {};
    rec.t = root;
    rec.p = r.at(rec.t);
    auto outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);

    return rec;
  }

  template <concepts::arithmetic U, std::uniform_random_bit_generator Gen>
  constexpr std::optional<std::pair<color3<U>, ray<T>>> scatter_impl(
      const ray<T>& r, const hit_record<T>& rec, Gen& gen) const {
        return material.scatter(r, rec, gen);
      }
};

template <concepts::arithmetic T, concepts::material M>
sphere(pos3<T, world_tag>, T, M) -> sphere<T, M>;

}  // namespace yk

#endif  // !YK_RAYTRACING_SPHERE_H

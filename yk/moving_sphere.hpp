#pragma once
#ifndef YK_RAYTRACING_MOVING_SPHERE_H
#define YK_RAYTRACING_MOVING_SPHERE_H

#include <random>

#include "aabb.hpp"
#include "concepts.hpp"
#include "hittable.hpp"
#include "material.hpp"
#include "raytracer.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T, concepts::material M>
struct moving_sphere : public hittable_interface<T, moving_sphere<T, M>> {
  pos3<T, world_tag> center0, center1;
  T time0, time1;
  T radius;
  M material;

  constexpr moving_sphere(pos3<T, world_tag> center0,
                          pos3<T, world_tag> center1, T time0, T time1,
                          T radius, M material) noexcept
      : center0(center0),
        center1(center1),
        time0(time0),
        time1(time1),
        radius(radius),
        material(material) {}

  constexpr pos3<T, world_tag> center(T time) const {
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
  }

  constexpr bool hit_impl(const ray<T>& r, T t_min, T t_max,
                          hit_record<T>& rec) const noexcept {
    vec3<T> oc = r.origin - center(r.time);
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
    auto outward_normal = (rec.p - center(r.time)) / radius;
    rec.set_face_normal(r, outward_normal);

    return true;
  }

  constexpr bool bouding_box_impl(T time0, T time1,
                                  aabb<T>& output_box) const noexcept {
    aabb<T> box0{
        center(time0) - vec3<T>(radius, radius, radius),
        center(time0) + vec3<T>(radius, radius, radius),
    };
    aabb<T> box1{
        center(time1) - vec3<T>(radius, radius, radius),
        center(time1) + vec3<T>(radius, radius, radius),
    };
    output_box = surrouding_box(box0, box1);
    return true;
  }

  template <concepts::arithmetic U, std::uniform_random_bit_generator Gen>
  constexpr bool scatter_impl(const ray<T>& r, const hit_record<T>& rec,
                              color3<U>& attenuation, ray<T>& scattered,
                              Gen& gen) const noexcept {
    return material.scatter(r, rec, attenuation, scattered, gen);
  }
};

template <concepts::arithmetic T, concepts::material M>
moving_sphere(pos3<T, world_tag>, pos3<T, world_tag>, T, T, T, M)
    -> moving_sphere<T, M>;

}  // namespace yk

#endif  // !YK_RAYTRACING_MOVING_SPHERE_H

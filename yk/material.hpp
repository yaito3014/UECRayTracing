#pragma once

#ifndef YK_RAYTRACING_MATERIAL_H
#define YK_RAYTRACING_MATERIAL_H

#include <functional>
#include <optional>
#include <random>
#include <utility>

#include "color.hpp"
#include "concepts.hpp"
#include "hittable.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

namespace concepts {

template <class T>
concept material = true;

}  // namespace concepts

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_in_unit_sphere(Gen& gen) {
  return vec3<T>::random(gen, -1, 1).normalize() *
         uniform_real_distribution<T>(0.01, 0.99)(gen);
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_unit_vector(Gen& gen) {
  return vec3<T>::random(gen, -1, 1).normalize();
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_in_hemisphere(const vec3<T>& normal, Gen& gen) {
  vec3<T> in_unit_sphere = random_unit_vector<T>(gen);
  return dot(in_unit_sphere, normal) > 0.0 ? in_unit_sphere : -in_unit_sphere;
}

template <concepts::arithmetic U>
struct lambertian {
  color3<U> albedo;

  constexpr lambertian(const color3<U>& albedo) : albedo(albedo) {}

  template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
  constexpr std::optional<std::pair<color3<U>, ray<T>>> scatter(
      const ray<T>&, const hit_record<T>& rec, Gen& gen) const {
    auto scatter_direction = rec.normal + random_unit_vector<T>(gen);

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero()) scatter_direction = rec.normal;

    return std::make_pair(albedo, ray(rec.p, scatter_direction));
  }
};

template <concepts::arithmetic U>
struct metal {
  color3<U> albedo;
  constexpr metal(const color3<U>& a) : albedo(a) {}

  template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
  constexpr std::optional<std::pair<color3<U>, ray<T>>> scatter(
      const ray<T>& r_in, const hit_record<T>& rec, Gen&) const {
    auto reflected = reflect(r_in.direction.normalized(), rec.normal);
    auto scattered = ray(rec.p, reflected);
    if (dot(scattered.direction, rec.normal) > 0)
      return std::make_pair(albedo, scattered);
    return std::nullopt;
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_MATERIAL_H

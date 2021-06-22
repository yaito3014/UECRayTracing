#pragma once

#ifndef YK_RAYTRACING_MATERIAL_H
#define YK_RAYTRACING_MATERIAL_H

#include <algorithm>
#include <functional>
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

template <concepts::arithmetic U>
struct lambertian {
  color3<U> albedo;

  constexpr lambertian(const color3<U>& albedo) noexcept : albedo(albedo) {}

  template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
  constexpr bool scatter(const ray<T>&, const hit_record<T>& rec,
                         color3<U>& attenuation, ray<T>& scattered,
                         Gen& gen) const noexcept {
    auto scatter_direction = rec.normal + random_unit_vector<T>(gen);

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero()) scatter_direction = rec.normal;

    attenuation = albedo;
    scattered = ray(rec.p, scatter_direction);
    return true;
  }
};

template <concepts::arithmetic U>
struct metal {
  color3<U> albedo;
  U fuzz;
  constexpr metal(const color3<U>& albedo, U fuzz) noexcept
      : albedo(albedo), fuzz(std::clamp<U>(fuzz, 0, 1)) {}

  template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
  constexpr bool scatter(const ray<T>& r_in, const hit_record<T>& rec,
                         color3<U>& attenuation, ray<T>& scattered,
                         Gen& gen) const {
    auto reflected = reflect(r_in.direction.normalized(), rec.normal);
    attenuation = albedo;
    scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere<T>(gen));
    return dot(scattered.direction, rec.normal) > 0;
  }
};

template <concepts::arithmetic U, concepts::arithmetic S = double>
struct dielectric {
  S ir;
  constexpr dielectric(S index_of_refraction) noexcept
      : ir(index_of_refraction) {}

  template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
  constexpr bool scatter(const ray<T>& r_in, const hit_record<T>& rec,
                         color3<U>& attenuation, ray<T>& scattered,
                         Gen& gen) const noexcept {
    auto refraction_ratio = rec.front_face ? 1 / ir : ir;
    auto unit_direction = r_in.direction.normalized();
    auto cos_theta = std::min(dot(-unit_direction, rec.normal), 1.);
    auto sin_theta = math::sqrt(1 - cos_theta * cos_theta);
    bool cannot_refract = refraction_ratio * sin_theta > 1;

    auto refractance = [](concepts::arithmetic auto cosine,
                          concepts::arithmetic auto ref_idx) noexcept {
      auto r0 = (1 - ref_idx) / (1 + ref_idx);
      r0 = r0 * r0;
      return r0 + (1 - r0) * math::pow((1 - cosine), 5);
    };

    uniform_real_distribution<S> dist(0, 1);

    auto direction =
        cannot_refract || refractance(cos_theta, refraction_ratio) > dist(gen)
            ? reflect(unit_direction, rec.normal)
            : refract(unit_direction, rec.normal, refraction_ratio);
    attenuation = color3<U>(1, 1, 1);
    scattered = ray(rec.p, direction);
    return true;
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_MATERIAL_H

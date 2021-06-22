#pragma once

#ifndef YK_RAYTRACING_RAYTRACER_H

#include <iostream>
#include <limits>
#include <type_traits>

#include "concepts.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T, concepts::arithmetic U>
struct raytracer {
  template <concepts::hittable<T> H, std::uniform_random_bit_generator Gen>
  constexpr color3<U> ray_color(const ray<T>& r, const H& world,
                                unsigned int depth, Gen& gen, color3<U> attenuation = {1, 1, 1}) const {
    if (!std::is_constant_evaluated() && verbose > 2)
      std::cout << "ray { origin : (" << r.origin.x << ", " << r.origin.y
                << ", " << r.origin.z << "), direction : (" << r.direction.x
                << ", " << r.direction.y << ", " << r.direction.z << ") }"
                << '\n';
    if (depth == 0) return color3<U>(0, 0, 0);
    if (auto rec = world.hit(r, 0.001, std::numeric_limits<T>::infinity());
        rec) {
      if (auto opt = world.template scatter<U>(r, rec.value(), gen); opt) {
        const auto& [att, scattered] = opt.value();
        return ray_color(scattered, world, depth - 1, gen, att * attenuation);
      } else
        return color3<U>(0, 0, 0);
    }
    auto t = (r.direction.normalized().y + 1.0) / 2;
    return ((1.0 - t) * color3<U>(1.0, 1.0, 1.0) + t * color3<U>(0.5, 0.7, 1.0)) * attenuation;
  }
};

}  // namespace yk

#define YK_RAYTRACING_RAYTRACER_H
#endif  // !YK_RAYTRACING_RAYTRACER_H

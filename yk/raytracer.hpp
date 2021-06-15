#pragma once

#ifndef YK_RAYTRACING_RAYTRACER_H

#include <iostream>
#include <limits>
#include <type_traits>

#include "concepts.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include "../thirdparty/harmony.hpp"

namespace yk {

template <concepts::arithmetic T, concepts::arithmetic U>
struct raytracer {

  template <concepts::hittable<T> H, std::uniform_random_bit_generator Gen>
  constexpr color3<U> ray_color(const ray<T>& r, const H& world,
                            unsigned int depth) {
                              using harmony::monadic_op;
    if (!std::is_constant_evaluated() && verbose > 2)
      std::cout << "ray { origin : (" << r.origin.x << ", " << r.origin.y
                << ", " << r.origin.z << "), direction : (" << r.direction.x
                << ", " << r.direction.y << ", " << r.direction.z << ") }"
                << '\n';
    if (depth == 0) return color3<U>(0, 0, 0);
    world.hit(r, 0.001, std::numeric_limits<T>::infinity())) | and_then([](const hit_record<T>& rec){

      // pos3<T, world_tag> target = rec.p + random_in_hemisphere<T>(rec.normal, gen);
    });
    auto t = (r.direction.normalized().y + 1.0) / 2;
    return (1.0 - t) * color3<U>(1.0, 1.0, 1.0) + t * color3<U>(0.5, 0.7, 1.0);
  }

};

}  // namespace yk

#define YK_RAYTRACING_RAYTRACER_H
#endif  // !YK_RAYTRACING_RAYTRACER_H

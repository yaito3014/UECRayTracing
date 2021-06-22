#pragma once

#ifndef YK_RAYTRACING_CAMERA_H
#define YK_RAYTRACING_CAMERA_H

#include <numbers>

#include "concepts.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

struct camera_tag : world_tag {};

template <concepts::arithmetic T>
struct camera {
  constexpr camera(pos3<T, world_tag> lookfrom, pos3<T, world_tag> lookat,
                   vec3<T> vup, T vfov, T aspect_ratio) {
    auto theta = vfov * std::numbers::pi / 180;
    auto h = math::tan(theta / 2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    auto w = (lookfrom - lookat).normalized();
    auto u = cross(vup, w).normalized();
    auto v = cross(w, u);

    origin = lookfrom;
    horizontal = viewport_width * u;
    vertical = viewport_height * v;
    lower_left_corner =
        pos3<T, camera_tag>(0, 0, 0) - horizontal / 2 - vertical / 2 - w;
  }

  constexpr ray<T> get_ray(T u, T v) const {
    return ray<T>(origin, (lower_left_corner + u * horizontal + v * vertical)
                              .template to<default_tag>());
  }

  pos3<T, world_tag> origin;
  pos3<T, camera_tag> lower_left_corner;
  vec3<T> horizontal;
  vec3<T> vertical;
};

}  // namespace yk

#endif  // !YK_RAYTRACING_CAMERA_H

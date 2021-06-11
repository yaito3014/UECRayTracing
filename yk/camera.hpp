#pragma once

#ifndef YK_RAYTRACING_CAMERA_H
#define YK_RAYTRACING_CAMERA_H

#include "concepts.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

struct camera_tag : world_tag {};

template <concepts::arithmetic T>
struct camera {
  constexpr camera() {
    auto aspect_ratio = 16.0 / 9.0;
    auto viewport_height = 2.0;
    auto viewport_width = aspect_ratio * viewport_height;
    auto focal_length = 1.0;

    origin = pos3<T, world_tag>(0, 0, 0);
    horizontal = vec3<T>(viewport_width, 0.0, 0.0);
    vertical = vec3<T>(0.0, viewport_height, 0.0);
    lower_left_corner = pos3<T, camera_tag>(0, 0, 0) - horizontal / 2 -
                        vertical / 2 - vec3<T>(0, 0, focal_length);
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

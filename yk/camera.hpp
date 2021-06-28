#pragma once

#ifndef YK_RAYTRACING_CAMERA_H
#define YK_RAYTRACING_CAMERA_H

#include <numbers>

#include "concepts.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

struct camera_tag : world_tag {};

template <concepts::arithmetic T>
struct camera {
  constexpr camera(pos3<T, world_tag> lookfrom, pos3<T, world_tag> lookat,
                   vec3<T> vup, T vfov, T aspect_ratio, T aperture,
                   T focus_dist, T time0, T time1) noexcept
      : time0(time0), time1(time1) {
    auto theta = vfov * std::numbers::pi / 180;
    auto h = math::tan(theta / 2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    w = (lookfrom - lookat).normalized();
    u = cross(vup, w).normalized();
    v = cross(w, u);

    origin = lookfrom;
    horizontal = focus_dist * viewport_width * u;
    vertical = focus_dist * viewport_height * v;
    lower_left_corner = pos3<T, camera_tag>(0, 0, 0) - horizontal / 2 -
                        vertical / 2 - focus_dist * w;
    lens_radius = aperture / 2;
  }

  template <std::uniform_random_bit_generator Gen>
  constexpr ray<T> get_ray(T s, T t, Gen& gen) const noexcept {
    auto rd = lens_radius * random_in_unit_disk<T>(gen);
    auto offset = u * rd.x + v * rd.y;
    return ray<T>(origin + offset,
                  (lower_left_corner + s * horizontal + t * vertical - offset)
                      .template to<default_tag>(),
                  uniform_real_distribution<T>(time0, time1)(gen));
  }

  pos3<T, world_tag> origin;
  pos3<T, camera_tag> lower_left_corner;
  vec3<T> horizontal;
  vec3<T> vertical;
  vec3<T> w, u, v;
  T lens_radius;
  T time0, time1;
};

}  // namespace yk

#endif  // !YK_RAYTRACING_CAMERA_H

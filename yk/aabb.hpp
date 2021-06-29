#pragma once

#ifndef YK_RAYTRACING_AABB_H
#define YK_RAYTRACING_AABB_H

#include <algorithm>
#include <functional>
#include <utility>

#include "concepts.hpp"
#include "ray.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T>
struct aabb {
  pos3<T, world_tag> min;
  pos3<T, world_tag> max;

  constexpr bool hit_impl(const ray<T>& r, T t_min, T t_max) const noexcept {
    auto pred = [&](auto proj) noexcept {
      auto invD = 1.0 / std::invoke(proj, r.direction);
      auto t0 = (std::invoke(proj, min) - std::invoke(proj, r.origin)) * invD;
      auto t1 = (std::invoke(proj, max) - std::invoke(proj, r.origin)) * invD;
      if (invD < 0) std::swap(t0, t1);
      t_min = std::max(t_min, t0);
      t_max = std::min(t_max, t1);
      return t_min < t_max;
    };
    return pred(cpo::x) && pred(cpo::y) && pred(cpo::z);
  }
};

template <concepts::arithmetic T>
aabb(pos3<T, world_tag>, pos3<T, world_tag>) -> aabb<T>;

template <concepts::arithmetic T>
aabb<T> surrounding_box(const aabb<T>& box0, const aabb<T>& box1) {
  return {
      .min =
          {
              std::min(box0.min.x, box1.min.x),
              std::min(box0.min.y, box1.min.y),
              std::min(box0.min.z, box1.min.z),
          },
      .max =
          {
              std::min(box0.max.x, box1.max.x),
              std::min(box0.max.y, box1.max.y),
              std::min(box0.max.z, box1.max.z),
          },
  };
}

}  // namespace yk

#endif  // !YK_RAYTRACING_AABB_H

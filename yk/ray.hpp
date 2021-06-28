#pragma once

#ifndef YK_RAYTRACING_RAY_H
#define YK_RAYTRACING_RAY_H

#include "concepts.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T>
struct ray {
  pos3<T, world_tag> origin;
  vec3<T> direction;
  T time = 0;

  template <concepts::arithmetic U>
  constexpr pos3<T, world_tag> at(U scalar) const noexcept {
    return origin + direction * scalar;
  }
};

template <concepts::arithmetic T>
ray(pos3<T, world_tag>, vec3<T>) -> ray<T>;

}  // namespace yk

#endif  // !YK_RAYTRACING_RAY_H

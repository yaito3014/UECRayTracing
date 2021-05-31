#pragma once

#ifndef YK_RAYTRACING_RAY_H
#define YK_RAYTRACING_RAY_H

#include "concepts.hpp"
#include "vec3.hpp"

namespace yk {

template <concepts::arithmetic T>
struct ray {
  pos3<T> origin;
  vec3<T> direction;

  template <concepts::arithmetic U>
  constexpr pos3<T> at(U scalar) const {
    return origin + direction * scalar;
  }
};

template <concepts::arithmetic T>
ray(pos3<T>, vec3<T>) -> ray<T>;

}  // namespace yk

#endif  // !YK_RAYTRACING_RAY_H

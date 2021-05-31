#pragma once

#ifndef YK_RAYTRACING_CONCEPTS_H
#define YK_RAYTRACING_CONCEPTS_H

#include <type_traits>

namespace yk::concepts {

template <class T>
concept arithmetic = std::is_arithmetic_v<T>;

}  // namespace yk::concepts

#endif  // !YK_RAYTRACING_CONCEPTS_H

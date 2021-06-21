#pragma once

#ifndef YK_RAYTRACING_CONFIG_H
#define YK_RAYTRACING_CONFIG_H

#ifndef YK_ENABLE_CONSTEXPR
#define YK_ENABLE_CONSTEXPR 0
#endif  // !YK_ENABLE_CONSTEXPR

#ifndef YK_ENABLE_PARALLEL
#define YK_ENABLE_PARALLEL 0
#endif  // !YK_ENABLE_PARALLEL

#if YK_ENABLE_CONSTEXPR

#if YK_ENABLE_PARALLEL
#error "YK_ENABLE_PARALLEL is runtime feature!"
#endif  // YK_ENABLE_PARALLEL

#define YK_CONSTEXPR constexpr

#else

#define YK_CONSTEXPR

#endif  // YK_ENABLE_CONSTEXPR

namespace yk {
  std::uint32_t verbose = 0;
}

#endif  // !YK_RAYTRACING_CONFIG_H
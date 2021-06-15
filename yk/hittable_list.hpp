#pragma once
#ifndef YK_RAYTRACING_HITTABLE_LIST_H
#define YK_RAYTRACING_HITTABLE_LIST_H

#include <cstddef>
#include <limits>
#include <tuple>
#include <utility>

#include "../thirdparty/harmony.hpp"
#include "concepts.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "raytracer.hpp"

namespace yk {

template <concepts::arithmetic T, concepts::hittable<T>... Hs>
struct hittable_list : public hittable_interface<T, hittable_list<T, Hs...>> {
  std::tuple<Hs...> objects = {};
  size_t hit_index = std::numeric_limits<size_t>::max();

  constexpr hittable_list() = default;
  constexpr hittable_list(std::tuple<Hs...> objects_)
      : objects(std::move(objects_)) {}

  template <concepts::hittable<T> H, concepts::hittable<T>... Args>
  [[nodiscard]] constexpr hittable_list<T, Hs..., H> add(H h, Args... args) {
    return hittable_list<T, Hs..., H>(std::tuple_cat(
        std::move(objects), std::tuple(std::move(h), std::move(args)...)));
  }

  constexpr std::optional<hit_record<T>> hit_impl(const ray<T>& r, T t_min,
                                                  T t_max) {
    using namespace harmony::monadic_op;
    auto closest_so_far = t_max;
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      auto pred = [&](concepts::hittable<T> auto h, size_t index) {
        return h.hit(r, t_min, closest_so_far) |
               and_then([&](const hit_record<T>& rec) {
                 closest_so_far = rec.t;
                 hit_index = index;
                 return rec;
               });
      };
      return (std::nullopt | ... | or_else([](std::nullopt_t) {
                return pred(std::get<Is>(objects, Is));
              }));
    }
    (std::index_sequence_for<Hs...>());
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_LIST_H

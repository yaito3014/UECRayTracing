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

  constexpr hittable_list() = default;
  constexpr hittable_list(std::tuple<Hs...> objects_)
      : objects(std::move(objects_)) {}

  template <concepts::hittable<T> H, concepts::hittable<T>... Args>
  [[nodiscard]] constexpr hittable_list<T, Hs..., H> add(H h, Args... args) {
    return hittable_list<T, Hs..., H>(std::tuple_cat(
        std::move(objects), std::tuple(std::move(h), std::move(args)...)));
  }

  constexpr std::optional<hit_record<T>> hit_impl(const ray<T>& r, T t_min,
                                                  T t_max) const {
    using namespace harmony::monadic_op;
    auto closest_so_far = t_max;
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      auto op = [&](concepts::hittable<T> auto h, std::size_t index) {
        return h.hit(r, t_min, closest_so_far) |
               and_then([&](hit_record<T> rec) {
                 closest_so_far = rec.t;
                 rec.id = index;
                 return std::optional(rec);
               });
      };
      return (std::optional<hit_record<T>>{} | ... |
              fold(
                  [&](const hit_record<T>& rec) {
                    return *(op(std::get<Is>(objects), Is) |
                             or_else([&](std::nullopt_t) {
                               return std::optional(rec);
                             }));
                  },
                  [&](std::nullopt_t) -> std::optional<hit_record<T>> {
                    return op(std::get<Is>(objects), Is);
                  }));
    }
    (std::index_sequence_for<Hs...>());
  }

  template <concepts::arithmetic U, std::uniform_random_bit_generator Gen>
  constexpr std::optional<std::pair<color3<U>, ray<T>>> scatter(
      const ray<T>& r, const hit_record<T>& rec, Gen& gen) const {
    using namespace harmony::monadic_op;
    return [&]<size_t... Is>(std::index_sequence<Is...>) {
      return (std::optional<std::pair<color3<U>, ray<T>>>{} | ... |
              or_else([&](std::nullopt_t) {
                return rec.id == Is ? std::get<Is>(objects).template scatter<U>(
                                          r, rec, gen)
                                    : std::nullopt;
              }));
    }
    (std::index_sequence_for<Hs...>());
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_LIST_H

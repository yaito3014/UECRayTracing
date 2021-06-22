#pragma once
#ifndef YK_RAYTRACING_HITTABLE_LIST_H
#define YK_RAYTRACING_HITTABLE_LIST_H

#include <cstddef>
#include <limits>
#include <tuple>
#include <utility>

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

  constexpr bool hit_impl(const ray<T>& r, T t_min, T t_max,
                          hit_record<T>& rec) const {
    hit_record<T> temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      auto op = [&](const concepts::hittable<T> auto& h, std::size_t index) {
        if (!h.hit(r, t_min, closest_so_far, temp_rec)) return false;
        closest_so_far = temp_rec.t;
        temp_rec.id = index;
        hit_anything = true;
      };
      void(op(std::get<Is>(objects), Is), ...);
      return hit_anything;
    }
    (std::index_sequence_for<Hs...>());
  }

  template <concepts::arithmetic U, std::uniform_random_bit_generator Gen>
  constexpr bool scatter(const ray<T>& r, const hit_record<T>& rec,
                         color3<U>& attenutation, ray<T>& scattered,
                         Gen& gen) const {
    return [&]<size_t... Is>(std::index_sequence<Is...>) {
      auto op = [&](const concepts::hittable<T> auto& h, std::size_t index) {
        return rec.id != index && h.scattered(r, rec, attenutation, scattered);
      };
      return (op(std::get<Is>(objects), Is) || ...);
    }
    (std::index_sequence_for<Hs...>());
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_LIST_H

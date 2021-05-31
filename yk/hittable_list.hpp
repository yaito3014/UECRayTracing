#pragma once
#ifndef YK_RAYTRACING_HITTABLE_LIST_H
#define YK_RAYTRACING_HITTABLE_LIST_H

#include <cstddef>
#include <tuple>
#include <utility>

#include "concepts.hpp"
#include "hittable.hpp"
#include "ray.hpp"

namespace yk {

template <concepts::arithmetic T, concepts::hittable<T>... Hs>
struct hittable_list : public hittable_interface<T, hittable_list<T, Hs...>> {
  std::tuple<Hs...> objects;

  template <concepts::hittable<T> H, concepts::hittable<T>... Args>
  [[nodiscard]] constexpr hittable_list<T, Hs..., H> add(H h, Args... args) {
    return hittable_list<T, Hs..., H>{
        .objects = std::tuple_cat(
            std::move(objects), std::tuple(std::move(h), std::move(args)...))};
  }

  constexpr bool hit_impl(const ray<T>& r, T t_min, T t_max,
                          hit_record<T>& rec) const {
    hit_record<T> temp_rec = {};
    bool hit_anything = false;
    auto closest_so_far = t_max;

    [&]<std::size_t... Is>(std::index_sequence<Is...>)->void {
      auto pred = [&](concepts::hittable<T> auto h) {
        if (!h.hit(r, t_min, closest_so_far, temp_rec)) return;
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
      };
      return (pred(std::get<Is>(objects)), ...);
    }
    (std::index_sequence_for<Hs...>());
    return hit_anything;
  }
};

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_LIST_H

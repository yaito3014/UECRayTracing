#pragma once
#ifndef YK_RAYTRACING_HITTABLE_LIST_H
#define YK_RAYTRACING_HITTABLE_LIST_H

#include <cstddef>
#include <limits>
#include <tuple>
#include <utility>

#include "aabb.hpp"
#include "concepts.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "raytracer.hpp"

namespace yk {

template <concepts::arithmetic T, concepts::hittable<T>... Hs>
struct hittable_list : public hittable_interface<T, hittable_list<T, Hs...>> {
  std::tuple<Hs...> objects = {};

  constexpr hittable_list() = default;
  constexpr hittable_list(std::tuple<Hs...> objects_) noexcept
      : objects(std::move(objects_)) {}

  template <class F>
  constexpr bool tuple_for_each(F op) const {
    return [ =, this ]<std::size_t... Is>(std::index_sequence<Is...>) noexcept {
      return (op(std::get<Is>(objects), Is) && ...);
    }
    (std::index_sequence_for<Hs...>());
  }

  constexpr bool hit_impl(const ray<T>& r, T t_min, T t_max,
                          hit_record<T>& rec) const noexcept {
    hit_record<T> temp_rec = {};
    bool hit_anything = false;
    auto closest_so_far = t_max;
    tuple_for_each(
        [&](const concepts::hittable<T> auto& h, std::size_t index) noexcept {
          if (h.hit(r, t_min, closest_so_far, temp_rec)) {
            closest_so_far = temp_rec.t;
            temp_rec.id = index;
            rec = temp_rec;
            hit_anything = true;
          }
          return true;  // continue iterating
        });
    return hit_anything;
  }

  constexpr bool bouding_box_impl(T time0, T time1,
                                  aabb<T>& output_box) const noexcept {
    if (sizeof...(Hs) == 0) return false;
    aabb<T> temp_box;
    bool first_box = true;
    return tuple_for_each([&](const concepts::hittable<T> auto& h,
                              std::size_t index) {
      if (!h.bouding_box(time0, time1, temp_box)) return false;
      output_box = first_box ? temp_box : surrouding_box(output_box, temp_box);
      first_box = false;
      return true;  // continue iterating
    });
  }

  template <concepts::arithmetic U, std::uniform_random_bit_generator Gen>
  constexpr bool scatter(const ray<T>& r, const hit_record<T>& rec,
                         color3<U>& attenuation, ray<T>& scattered,
                         Gen& gen) const noexcept {
    bool is_scattered = false;
    tuple_for_each(
        [&](const concepts::hittable<T> auto& h, std::size_t index) noexcept {
          is_scattered =
              is_scattered ||
              rec.id == index && h.scatter(r, rec, attenuation, scattered, gen);
          return true;  // continue iterating
        });
    return is_scattered;
  }
};

template <concepts::arithmetic T, concepts::hittable<T>... Hs, class H>
requires concepts::hittable<std::remove_cvref_t<H>, T>
[[nodiscard]] constexpr hittable_list<T, Hs..., std::remove_cvref_t<H>>
operator|(hittable_list<T, Hs...>&& lhs, H&& rhs) noexcept {
  return {std::tuple_cat(std::move(lhs.objects),
                         std::make_tuple(std::forward<H>(rhs)))};
}

template <concepts::arithmetic T, concepts::hittable<T>... H1s,
          concepts::hittable<T>... H2s>
[[nodiscard]] constexpr hittable_list<T, H1s..., H2s...> operator|(
    hittable_list<T, H1s...>&& lhs, hittable_list<T, H2s...>&& rhs) noexcept {
  return {std::tuple_cat(std::move(lhs.objects), std::move(rhs.objects))};
}

}  // namespace yk

#endif  // !YK_RAYTRACING_HITTABLE_LIST_H

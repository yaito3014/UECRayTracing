#pragma once

#ifndef YK_RAYTRACING_BVH_H
#define YK_RAYTRACING_BVH_H

#include <numeric>
#include <tuple>
#include <utility>

#include "aabb.hpp"
#include "concepts.hpp"
#include "hittable.hpp"
#include "hittable_list.hpp"

namespace yk {

namespace detail {

template <std::size_t, class... Ts>
constexpr auto split_tuple_impl(std::tuple<Ts...> lhs, std::tuple<> rhs,
                                std::index_sequence<>) {
  return std::make_pair(lhs, rhs);
}

template <std::size_t M, class... Ts, class U, class... Us, std::size_t... Is>
constexpr auto split_tuple_impl(std::tuple<Ts...> lhs, std::tuple<U, Us...> rhs,
                                std::index_sequence<Is...>) {
  if constexpr (sizeof...(Ts) == M) {
    return std::make_pair(lhs, rhs);
  } else {
    return split_tuple_impl<M>(
        std::tuple_cat(std::move(lhs), std::tuple<U>(std::get<0>(rhs))),
        std::tuple(std::get<Is + 1>(rhs)...),
        std::make_index_sequence<sizeof...(Is) - 1>());
  }
}

template <class... Ts>
constexpr auto split_tuple(std::tuple<Ts...> tpl) {
  return split_tuple_impl<sizeof...(Ts) / 2>(std::tuple{}, tpl,
                                             std::make_index_sequence<sizeof...(Ts) - 1>());
}

template <class... Ts, class... Us, class... Vs>
constexpr auto merge_tuple_impl(std::tuple<Ts...> lhs, std::tuple<Us...> rhs, std::tuple<Vs...> tmp) {

}

template <class... Ts, class... Us>
constexpr auto merge_tuple(std::tuple<Ts...> lhs, std::tuple<Us...> rhs) {

}


}  // namespace detail

template <concepts::arithmetic T, concepts::hittable<T> Left,
          concepts::hittable<T> Right>
struct bvh_node : hittable_interface<T, bvh_node<T, Left, Right>> {
  aabb<T> box;
  Left left;
  Right right;

  template <concepts::hittable<T>... Hs>
  constexpr bvh_node(const hittable_list<T>& list, T time0, T time1)
    : bvh_node(list.objects, time0, time1) {}

  template <concepts::hittable<T>... Hs>
  constexpr bvh_node(const std::tuple<Hs...> objects, T time0, T time1) {}
};

}  // namespace yk

#endif  // !YK_RAYTRACING_BVH_H

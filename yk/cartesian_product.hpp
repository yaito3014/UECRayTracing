#pragma once

#ifndef YK_RAYTRACING_CARTESIAN_PRODUCT_H
#define YK_RAYTRACING_CARTESIAN_PRODUCT_H

#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace yk {

namespace ranges {

template <bool Const, class V>
using maybe_const = std::conditional_t<Const, const V, V>;

template <class R>
concept simple_view = std::ranges::view<R> && std::ranges::range<const R> &&
    std::same_as<std::ranges::iterator_t<R>,
                 std::ranges::iterator_t<const R>> &&
    std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

template <class... Vs>
concept cartesian_product_is_random_access =
    ((std::ranges::random_access_range<Vs> && ...) &&
     (std::ranges::sized_range<Vs> && ...));

template <class... Vs>
concept cartesian_product_is_bidirectional =
    ((std::ranges::bidirectional_range<Vs> && ...) &&
     (std::ranges::common_range<Vs> && ...));

template <class... Vs>
concept cartesian_product_is_common = (std::ranges::common_range<Vs> && ...) ||
                                      cartesian_product_is_random_access<Vs...>;

// template <class... Ts>
// using tuple_or_pair = decltype([] {
//   if constexpr (sizeof...(Ts) == 2) {
//     return std::type_identity<std::pair<Ts...>>{};
//   } else {
//     return std::type_identity<std::tuple<Ts...>>{};
//   }
// }())::type;

template <class... Ts>
struct tuple_or_pair_impl {
  using type = std::tuple<Ts...>;
};

template <class T, class U>
struct tuple_or_pair_impl<T, U> {
  using type = std::pair<T, U>;
};

template <class... Ts>
using tuple_or_pair = typename tuple_or_pair_impl<Ts...>::type;

template <class F, class Tuple>
constexpr auto tuple_transform(F&& f, Tuple&& tuple) {
  return apply(
      [&]<class... Ts>(Ts && ... elements) {
        return tuple_or_pair<std::invoke_result_t<F&, Ts>...>(
            std::invoke(f, std::forward<Ts>(elements))...);
      },
      std::forward<Tuple>(tuple));
}

template <class F, class Tuple>
constexpr void tuple_for_each(F&& f, Tuple&& tuple) {
  apply(
      [&]<class... Ts>(Ts && ... elements) {
        (std::invoke(f, std::forward<Ts>(elements)), ...);
      },
      std::forward<Tuple>(tuple));
}

template <std::ranges::forward_range... Vs>
  requires(std::ranges::view<Vs>&&...)
class cartesian_product_view
    : public std::ranges::view_interface<cartesian_product_view<Vs...>> {
 private:
  std::tuple<Vs...> bases_;  // exposition only

  template <bool Const>
  class iterator {
    using Parent = maybe_const<Const, cartesian_product_view>;
    Parent* parent_;
    tuple_or_pair<std::ranges::iterator_t<maybe_const<Const, Vs>>...>
        current_{};  // exposition only

    template <size_t N = (sizeof...(Vs) - 1)>
    constexpr void next() {
      auto& it = std::get<N>(current_);
      ++it;
      if constexpr (N > 0) {
        if (it == std::ranges::end(std::get<N>(parent_->bases_))) {
          it = std::ranges::begin(std::get<N>(parent_->bases_));
          next<N - 1>();
        }
      }
    }

    template <size_t N = (sizeof...(Vs) - 1)>
    constexpr void prev() {
      auto& it = std::get<N>(current_);
      if (it == std::ranges::begin(std::get<N>(parent_->bases_))) {
        std::ranges::advance(it,
                             std::ranges::end(std::get<N>(parent_->bases_)));
        if constexpr (N > 0) {
          prev<N - 1>();
        }
      }
      --it;
    }

   public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::conditional_t<
        (std::ranges::random_access_range<Vs> && ...),
        std::random_access_iterator_tag,
        std::conditional_t<(std::ranges::bidirectional_range<Vs> && ...),
                           std::bidirectional_iterator_tag,
                           std::forward_iterator_tag>>;
    using value_type =
        tuple_or_pair<std::ranges::range_value_t<maybe_const<Const, Vs>>...>;
    using difference_type = std::common_type_t<
        std::ranges::range_difference_t<maybe_const<Const, Vs>>...>;

    iterator() = default;
    constexpr explicit iterator(
        Parent& parent,
        tuple_or_pair<std::ranges::iterator_t<maybe_const<Const, Vs>>...>
            current)
        : parent_(std::addressof(parent)), current_(std::move(current)) {}

    constexpr iterator(iterator<!Const> i) requires Const &&
        (std::convertible_to<
            std::ranges::iterator_t<Vs>,
            std::ranges::iterator_t<maybe_const<Const, Vs>>>&&...)
        : current_(std::move(i.current_)) {}

    constexpr auto operator*() const {
      return tuple_transform([](auto& i) -> decltype(auto) { return *i; },
                             current_);
    }
    constexpr iterator& operator++() {
      next();
      return *this;
    }
    constexpr iterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--() requires(
        cartesian_product_is_bidirectional<maybe_const<Const, Vs>...>) {
      prev();
      return *this;
    }
    constexpr iterator operator--(int) requires(
        cartesian_product_is_bidirectional<maybe_const<Const, Vs>...>) {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    // TODO: implement!
    constexpr iterator& operator+=(difference_type x) requires(
        cartesian_product_is_random_access<maybe_const<Const, Vs>...>) {
      auto advance = [this]<size_t N = sizeof...(Vs)>(auto& self,
                                                      difference_type n) {
        static_assert(N != 0);
        if (n == 0) return;
        auto& i = std::get<N - 1>(current_);
        auto const my_size = static_cast<difference_type>(
            std::ranges::size(std::get<N - 1>(parent_->bases_)));
        auto const first = std::ranges::begin(std::get<N - 1>(parent_->bases_));
        auto const idx = static_cast<difference_type>(i - first);
        n += idx;
        auto n_div = n / my_size;
        auto n_mod = n % my_size;

        if constexpr (N != 1) {
          if (n_mod < 0) {
            n_mod += my_size;
            --n_div;
          }
          self.template operator()<N - 1>(self, n_div);
        }

        if constexpr (N == 1)
          if (n_div > 0) n_mod = my_size;

        using D = std::iter_difference_t<decltype(first)>;
        i = first + static_cast<D>(n_mod);
      };
      advance(advance, x);
      return *this;
    }
    constexpr iterator& operator-=(difference_type x) requires(
        cartesian_product_is_random_access<maybe_const<Const, Vs>...>) {
      *this += -x;
      return *this;
    }

    constexpr auto operator[](difference_type n) const requires(
        cartesian_product_is_random_access<maybe_const<Const, Vs>...>) {
      return *((*this) + n);
    }

    friend constexpr bool
    operator==(const iterator& x, const iterator& y) requires(
        std::equality_comparable<
            std::ranges::iterator_t<maybe_const<Const, Vs>>>&&...) {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator==(const iterator& x,
                                     const std::default_sentinel_t&) {
      return false;
    }

    friend constexpr auto
    operator<(const iterator& x, const iterator& y) requires(
        std::ranges::random_access_range<maybe_const<Const, Vs>>&&...) {
      return x.current_ < y.current_;
    }
    friend constexpr auto
    operator>(const iterator& x, const iterator& y) requires(
        std::ranges::random_access_range<maybe_const<Const, Vs>>&&...) {
      return y < x;
    }
    friend constexpr auto
    operator<=(const iterator& x, const iterator& y) requires(
        std::ranges::random_access_range<maybe_const<Const, Vs>>&&...) {
      return !(y < x);
    }
    friend constexpr auto
    operator>=(const iterator& x, const iterator& y) requires(
        std::ranges::random_access_range<maybe_const<Const, Vs>>&&...) {
      return !(x < y);
    }

    friend constexpr auto
    operator<=>(const iterator& x, const iterator& y) requires(
        (std::ranges::random_access_range<maybe_const<Const, Vs>> && ...) &&
        (std::three_way_comparable<maybe_const<Const, Vs>> && ...)) {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator
    operator+(const iterator& x, difference_type y) requires(
        cartesian_product_is_random_access<maybe_const<Const, Vs>...>) {
      return iterator{x} += y;
    }
    friend constexpr iterator
    operator+(difference_type x, const iterator& y) requires(
        cartesian_product_is_random_access<maybe_const<Const, Vs>...>) {
      return y + x;
    }
    friend constexpr iterator
    operator-(const iterator& x, difference_type y) requires(
        cartesian_product_is_random_access<maybe_const<Const, Vs>...>) {
      return iterator{x} -= y;
    }
    friend constexpr difference_type
    operator-(const iterator& x, const iterator& y) requires(
        std::sized_sentinel_for<
            std::ranges::iterator_t<maybe_const<Const, Vs>>,
            std::ranges::iterator_t<maybe_const<Const, Vs>>>&&...) {
      auto scaled_distance = []<size_t N>(auto x, auto y) {
        return (std::get<N>(x.current_) - std::get<N>(y.current_)) *
               std::get<N>(x.parent_->bases_);
      };
      return [&]<size_t... Is>(std::index_sequence<Is...>) {
        return (0 + ... + (scaled_distance<sizeof...(Vs) - Is>(x, y)));
      }
      (std::index_sequence_for<Vs...>());
    }
  };
  template <bool Const>
  class sentinel {
    using parent = maybe_const<Const, cartesian_product_view>;
    using first_base = decltype(std::get<0>(std::declval<parent>().bases_));
    std::ranges::sentinel_t<first_base> end_;

   public:
    sentinel() = default;
    sentinel(std::ranges::sentinel_t<first_base> end) : end_(std::move(end)) {}

    friend constexpr bool operator==(
        sentinel const& s, std::ranges::iterator_t<parent> const& it) {
      return std::get<0>(it.current_) == s.end_;
    }

    constexpr sentinel(sentinel<!Const> other) requires Const &&
        (std::convertible_to<std::ranges::sentinel_t<first_base>,
                             std::ranges::sentinel_t<const first_base>>)
        : end_(std::move(other.end_)) {}
  };

 public:
  constexpr cartesian_product_view() = default;
  constexpr cartesian_product_view(Vs... bases) : bases_(std::move(bases)...) {}

  constexpr iterator<false> begin() requires(!simple_view<Vs> || ...) {
    return iterator<false>(*this, tuple_transform(std::ranges::begin, bases_));
  }
  constexpr iterator<true> begin() const
      requires(std::ranges::range<const Vs>&&...) {
    return iterator<true>(*this, tuple_transform(std::ranges::begin, bases_));
  }

  constexpr iterator<false> end() requires((!simple_view<Vs> || ...) &&
                                           cartesian_product_is_common<Vs...>) {
    if constexpr ((std::ranges::random_access_range<Vs> && ...))
      return begin() + size();
    else {
      iterator<false> it(tuple_transform(std::ranges::begin, bases_));
      std::get<0>(it.current_) = std::ranges::end(std::get<0>(bases_));
      return it;
    }
  }

  constexpr iterator<true> end() const
      requires(cartesian_product_is_common<const Vs...>) {
    if constexpr ((std::ranges::random_access_range<Vs> && ...))
      return begin() + size();
    else {
      iterator<true> it(tuple_transform(std::ranges::begin, bases_));
      std::get<0>(it.current_) = std::ranges::end(std::get<0>(bases_));
      return it;
    }
  }
  constexpr std::default_sentinel_t end() const
      requires(!cartesian_product_is_common<const Vs...>) {
    return std::default_sentinel;
  }

  constexpr auto size() requires(std::ranges::sized_range<Vs>&&...) {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return (1u * ... * std::ranges::size(std::get<Is>(bases_)));
    }
    (std::index_sequence_for<Vs...>());
  }
  constexpr auto size() const
      requires(std::ranges::sized_range<const Vs>&&...) {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return (1u * ... * std::ranges::size(std::get<Is>(bases_)));
    }
    (std::index_sequence_for<Vs...>());
  }
};

template <class... Vs>
cartesian_product_view(Vs&&...)
    -> cartesian_product_view<std::views::all_t<Vs>...>;

namespace views {

namespace detail {

// True if the range adaptor Adaptor can be applied with Args.
template <typename Adaptor, typename... Args>
concept adaptor_invocable = requires {
  std::declval<Adaptor>()(std::declval<Args>()...);
};

template <typename Adaptor, typename... Args>
struct Partial;

template <typename Lhs, typename Rhs>
struct Pipe;

// The base class of every range adaptor closure.
struct RangeAdaptorClosure {
  // range | adaptor is equivalent to adaptor(range).
  template <typename Self, typename Range>
    requires std::derived_from<std::remove_cvref_t<Self>,
                               RangeAdaptorClosure> &&
        adaptor_invocable<Self, Range>
  friend constexpr auto operator|(Range&& r, Self&& self) {
    return std::forward<Self>(self)(std::forward<Range>(r));
  }

  // Compose the adaptors lhs and rhs into a pipeline, returning
  // another range adaptor closure object.
  template <typename Lhs, typename Rhs>
    requires std::derived_from<Lhs, RangeAdaptorClosure> &&
        std::derived_from<Rhs, RangeAdaptorClosure>
  friend constexpr auto operator|(Lhs lhs, Rhs rhs) {
    return Pipe<Lhs, Rhs>{std::move(lhs), std::move(rhs)};
  }
};

// A range adaptor closure that represents partial application of
// the range adaptor Adaptor with arguments Args.
template <typename Adaptor, typename... Args>
struct Partial : RangeAdaptorClosure {
  std::tuple<Args...> M_args;

  constexpr Partial(Args... args) : M_args(std::move(args)...) {}

  // Invoke Adaptor with arguments r, M_args... according to the
  // value category of the range adaptor closure object.
  template <typename Range>
    requires adaptor_invocable<Adaptor, Range, const Args&...>
  constexpr auto operator()(Range&& r) const& {
    auto forwarder = [&r](const auto&... args) {
      return Adaptor{}(std::forward<Range>(r), args...);
    };
    return std::apply(forwarder, M_args);
  }

  template <typename Range>
    requires adaptor_invocable<Adaptor, Range, Args...>
  constexpr auto operator()(Range&& r) && {
    auto forwarder = [&r](auto&... args) {
      return Adaptor{}(std::forward<Range>(r), std::move(args)...);
    };
    return std::apply(forwarder, M_args);
  }

  template <typename Range>
  constexpr auto operator()(Range&& r) const&& = delete;
};

// A lightweight specialization of the above primary template for
// the common case where Adaptor accepts a single extra argument.
template <typename Adaptor, typename Arg>
struct Partial<Adaptor, Arg> : RangeAdaptorClosure {
  Arg M_arg;

  constexpr Partial(Arg arg) : M_arg(std::move(arg)) {}

  template <typename Range>
    requires adaptor_invocable<Adaptor, Range, const Arg&>
  constexpr auto operator()(Range&& r) const& {
    return Adaptor{}(std::forward<Range>(r), M_arg);
  }

  template <typename Range>
    requires adaptor_invocable<Adaptor, Range, Arg>
  constexpr auto operator()(Range&& r) && {
    return Adaptor{}(std::forward<Range>(r), std::move(M_arg));
  }

  template <typename Range>
  constexpr auto operator()(Range&& r) const&& = delete;
};

template <typename Lhs, typename Rhs, typename Range>
concept pipe_invocable = requires {
  std::declval<Rhs>()(std::declval<Lhs>()(std::declval<Range>()));
};

// A range adaptor closure that represents composition of the range
// adaptor closures Lhs and Rhs.
template <typename Lhs, typename Rhs>
struct Pipe : RangeAdaptorClosure {
  [[no_unique_address]] Lhs M_lhs;
  [[no_unique_address]] Rhs M_rhs;

  constexpr Pipe(Lhs lhs, Rhs rhs)
      : M_lhs(std::move(lhs)), M_rhs(std::move(rhs)) {}

  // Invoke M_rhs(M_lhs(r)) according to the value category of this
  // range adaptor closure object.
  template <typename Range>
    requires pipe_invocable<const Lhs&, const Rhs&, Range>
  constexpr auto operator()(Range&& r) const& {
    return M_rhs(M_lhs(std::forward<Range>(r)));
  }

  template <typename Range>
    requires pipe_invocable<Lhs, Rhs, Range>
  constexpr auto operator()(Range&& r) && {
    return std::move(M_rhs)(std::move(M_lhs)(std::forward<Range>(r)));
  }

  template <typename Range>
  constexpr auto operator()(Range&& r) const&& = delete;
};

struct cartesian_product_adaptor : RangeAdaptorClosure {
  template <std::ranges::viewable_range... Ranges>
  constexpr auto operator()(Ranges&&... rs) const {
    return cartesian_product_view{std::forward<Ranges>(rs)...};
  }
};

}  // namespace detail

inline constexpr detail::cartesian_product_adaptor cartesian_product = {};

}  // namespace views

}  // namespace ranges

namespace views = ranges::views;

}  // namespace yk

#endif  // YK_RAYTRACING_CARTESIAN_PRODUCT_H

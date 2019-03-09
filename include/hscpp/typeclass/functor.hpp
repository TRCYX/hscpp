#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/operator_identifiers/dollar.inl>

template <ORDER_2_TYPE F, typename = void>
struct Functor;

template <ORDER_2_TYPE F, typename = void>
struct FunctorDetector : false_type {};

template <ORDER_2_TYPE F>
struct FunctorDetector<F, enable_if_t<sizeof(Functor<F>) >= 0, void>>
    : true_type {};

template <ORDER_2_TYPE F>
constexpr bool is_functor = FunctorDetector<F>::value;

template <typename T>
constexpr bool is_of_functor = false;
template <ORDER_2_TYPE F, typename A>
constexpr bool is_of_functor<F<A>> = is_functor<F>;

template <ORDER_2_TYPE F, typename A, typename Func,
          typename B = invoke_result_t<Func, A>>
inline constexpr F<B> fmap_(const Func &f, const F<A> &a) {
    static_assert(is_functor<F>);
    static_assert(
      is_same_v<remove_cvref_t<decltype(Functor<F>::fmapImpl(f, a))>, F<B>>);
    return Functor<F>::fmapImpl(f, a);
}

constexpr auto fmap =
  curry([](const auto &f, const auto &a) { return fmap_(f, a); });

DECLARE_BINARY_OP(fmap, <$>);

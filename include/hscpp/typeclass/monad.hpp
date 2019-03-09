#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/operator_identifiers/underscore.inl>
#include <hscpp/typeclass/applicative.hpp>

template <ORDER_2_TYPE M, typename = void_t<Applicative<M>>>
struct Monad;

template <ORDER_2_TYPE M, typename = void>
struct MonadDetector : false_type {};

template <ORDER_2_TYPE M>
struct MonadDetector<M, enable_if_t<sizeof(Monad<M>) >= 0, void>>
    : true_type {};

template <ORDER_2_TYPE M>
constexpr bool is_monad = is_applicative<M> &&MonadDetector<M>::value;

template <typename T>
constexpr bool is_of_monad = false;
template <ORDER_2_TYPE M, typename A>
constexpr bool is_of_monad<M<A>> = is_monad<M>;

constexpr auto returnM = pure;

template <ORDER_2_TYPE M, typename Func, typename A,
          typename B = get_param_t<M, invoke_result_t<Func, A>>>
inline constexpr M<B> bindM_(const Func &f, const M<A> &a) {
    static_assert(is_monad<M>);
    static_assert(is_same_v<invoke_result_t<Func, A>, M<B>>);
    static_assert(
      is_same_v<remove_cvref_t<decltype(Monad<M>::bindMImpl(f, a))>, M<B>>);
    return Monad<M>::bindMImpl(f, a);
}

constexpr auto bindM =
  curry([](const auto &f, const auto &a) { return bindM_(f, a); });

template <ORDER_2_TYPE M, typename F, typename A>
inline constexpr auto operator>>(const M<A> &a, const F &f) {
    return bindM_(f, a);
}

template <ORDER_2_TYPE M, typename F, typename A>
inline constexpr auto operator<<(const F &f, const M<A> &a) {
    return bindM_(f, a);
}

DECLARE_BINARY_OP(([](const auto &a, const auto &b) {
                      return
                        [a, b](const auto &x) -> decltype(bindM_(b, a(x))) {
                            return bindM_(b, a(x));
                        };
                  }), >_>);
DECLARE_BINARY_OP(([](const auto &a, const auto &b) {
                      return
                        [a, b](const auto &x) -> decltype(bindM_(a, b(x))) {
                            return bindM_(a, b(x));
                        };
                  }), <_<);

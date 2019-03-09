#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/operator_identifiers/Ap.inl>
#include <hscpp/operator_identifiers/M.inl>
#include <hscpp/typeclass/functor.hpp>

template <ORDER_2_TYPE F, typename = void_t<Functor<F>>>
struct Applicative;

template <ORDER_2_TYPE F, typename = void>
struct ApplicativeDetector : false_type {};

template <ORDER_2_TYPE F>
struct ApplicativeDetector<F, enable_if_t<sizeof(Applicative<F>) >= 0, void>>
    : true_type {};

template <ORDER_2_TYPE F>
constexpr bool is_applicative = is_functor<F> &&ApplicativeDetector<F>::value;

template <typename T>
constexpr bool is_of_applicative = false;
template <ORDER_2_TYPE F, typename A>
constexpr bool is_of_applicative<F<A>> = is_applicative<F>;

template <ORDER_2_TYPE F, typename A>
inline F<A> pure_(const A &a) {
    static_assert(is_applicative<F>);
    static_assert(
      is_same_v<remove_cvref_t<decltype(Applicative<F>::pureImpl(a))>, F<A>>);
    return Applicative<F>::pureImpl(a);
}

template <typename T>
struct pure_t {
    explicit constexpr pure_t(T value) : content(move(value)) {}

    template <
      ORDER_2_TYPE F, typename U,
      typename = enable_if_t<is_applicative<F> && is_convertible_v<T, U>>>
    operator F<U>() {
        return pure_<F>(static_cast<U &&>(move(content)));
    }

  private:
    T content;
};

constexpr auto pure = [](auto a) { return pure_t(move(a)); };

template <ORDER_2_TYPE F, typename Func, typename A,
          typename B = invoke_result_t<Func, A>>
inline constexpr F<B> ap_(const F<Func> &f, const F<A> &a) {
    static_assert(is_applicative<F>);
    static_assert(
      is_same_v<remove_cvref_t<decltype(Applicative<F>::apImpl(f, a))>,
                F<B>>);
    return Applicative<F>::apImpl(f, a);
}

constexpr auto ap =
  curry([](const auto &f, const auto &a) { return ap_(f, a); });

DECLARE_BINARY_OP(ap, <Ap>);

constexpr auto take_right =
  curry([](const auto &a, const auto &b) { return seq <$> a <Ap> b; });

constexpr auto take_left =
  curry([](const auto &a, const auto &b) { return konst <$> a <Ap> b; });

DECLARE_BINARY_OP(take_right, *Ap>);
DECLARE_BINARY_OP(take_left, <Ap*);
DECLARE_BINARY_OP(take_right, >M>);
DECLARE_BINARY_OP(take_left, <M<);

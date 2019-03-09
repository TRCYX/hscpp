#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/typeclass/applicative.hpp>
#include <hscpp/typeclass/foldable.hpp>
#include <hscpp/typeclass/functor.hpp>

template <ORDER_2_TYPE T>
struct Traversable;

template <ORDER_2_TYPE T, typename = void>
struct TraversableDetector : false_type {};

template <ORDER_2_TYPE T>
struct TraversableDetector<T, enable_if_t<sizeof(Traversable<T>) >= 0, void>>
    : true_type {};

template <ORDER_2_TYPE T>
constexpr bool is_traversable = TraversableDetector<T>::value;

template <typename T>
constexpr bool is_of_traversable = false;
template <ORDER_2_TYPE T, typename A>
constexpr bool is_of_traversable<T<A>> = is_traversable<T>;

template <ORDER_2_TYPE T, ORDER_2_TYPE F, typename A>
constexpr F<T<A>> sequence_(const T<F<A>> &);

template <
  ORDER_2_TYPE T, typename Func, typename A,
  typename F_B = remove_cvref_t<invoke_result_t<Func, A>>,
  ORDER_2_TYPE F = constructor_param_separator<F_B>::template constructor_t,
  typename B = typename constructor_param_separator<F_B>::param_t>
inline constexpr F<T<B>> traverse_(const Func &f, const T<A> &t) {
    static_assert(is_traversable<T>);
    static_assert(is_of_applicative<F_B>);

    constexpr Validator validator(
      [](auto traversable, auto f,
         auto t) -> decltype(decltype(traversable)::traverseImpl(f, t)) {});
    if constexpr (validator.template is_valid<Traversable<T>, Func, T<A>>) {
        static_assert(
          is_same_v<
            remove_cvref_t<decltype(Traversable<T>::traverseImpl(f, t))>,
            F<T<B>>>);
        return Traversable<T>::traverseImpl(f, t);
    } else {
        return sequence_(fmap(f, t));
    }
}

constexpr auto traverse =
  curry([](const auto &f, const auto &t) { return traverse_(f, t); });

template <ORDER_2_TYPE T, ORDER_2_TYPE F, typename A>
inline constexpr F<T<A>> sequence_(const T<F<A>> &t) {
    static_assert(is_traversable<T>);
    static_assert(is_applicative<F>);

    constexpr Validator validator(
      [](auto traversable,
         auto t) -> decltype(decltype(traversable)::sequenceImpl(t)) {});
    if constexpr (validator.template is_valid<Traversable<T>, T<F<A>>>) {
        static_assert(
          is_same_v<remove_cvref_t<decltype(Traversable<T>::sequenceImpl(t))>,
                    F<T<A>>>);
        return Traversable<T>::sequenceImpl(t);
    } else {
        return traverse_(id, t);
    }
}

constexpr auto sequence = [](const auto &t) { return sequence_(t); };

constexpr auto mapM = traverse;
constexpr auto sequenceA = sequence;

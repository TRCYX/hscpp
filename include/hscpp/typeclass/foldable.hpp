#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/typeclass/monoid.hpp>
#include <hscpp/types/dual.hpp>
#include <hscpp/types/endo.hpp>

template <ORDER_2_TYPE T>
struct Foldable;

template <ORDER_2_TYPE T, typename = void>
struct FoldableDetector : false_type {};

template <ORDER_2_TYPE T>
struct FoldableDetector<T, enable_if_t<sizeof(Foldable<T>) >= 0, void>>
    : true_type {};

template <ORDER_2_TYPE T>
constexpr bool is_foldable = FoldableDetector<T>::value;

template <typename T>
constexpr bool is_of_foldable = false;
template <ORDER_2_TYPE T, typename A>
constexpr bool is_of_foldable<T<A>> = is_foldable<T>;

template <ORDER_2_TYPE T, typename Func, typename A, typename B>
constexpr B foldr_(const Func &, const B &, const T<A> &);

template <ORDER_2_TYPE T, typename Func, typename A,
          typename M = remove_cvref_t<invoke_result_t<Func, A>>>
inline constexpr M foldMap_(const Func &f, const T<A> &t) {
    static_assert(is_foldable<T>);
    static_assert(is_monoid<M>);

    constexpr Validator validator(
      [](auto foldable, auto f,
         auto t) -> decltype(decltype(foldable)::foldMapImpl(f, t)) {});
    if constexpr (validator.template is_valid<Foldable<T>, Func, T<A>>) {
        static_assert(
          is_same_v<remove_cvref_t<decltype(Foldable<T>::foldMapImpl(f, t))>,
                    M>);
        return Foldable<T>::foldMapImpl(f, t);
    } else {
        return foldr_(
          [f](const auto &x, const M &acc) { return mappend(f(x), acc); },
          mempty<M>, t);
    }
}

constexpr auto foldMap =
  curry([](const auto &f, const auto &t) { return foldMap_(f, t); });

template <ORDER_2_TYPE T, typename Func, typename A, typename B>
inline constexpr B foldr_(const Func &f, const B &acc, const T<A> &t) {
    static_assert(is_foldable<T>);
    static_assert(
      is_same_v<remove_cvref_t<decltype(f(declval<A>(), declval<B>()))>, B>);

    constexpr Validator validator(
      [](auto foldable, auto f, auto acc,
         auto t) -> decltype(decltype(foldable)::foldrImpl(f, acc, t)) {});
    if constexpr (validator.template is_valid<Foldable<T>, Func, B, T<A>>) {
        static_assert(
          is_same_v<
            remove_cvref_t<decltype(Foldable<T>::foldrImpl(f, acc, t))>, B>);
        return Foldable<T>::foldrImpl(f, acc, t);
    } else {
        return appEndo(foldMap_(
          [f](const auto &a) { return Endo<B>(curry(f)(a)); }, t))(acc);
    }
}

constexpr auto foldr = curry([](const auto &f, const auto &acc,
                                const auto &t) { return foldr_(f, acc, t); });

constexpr auto fold = [](const auto &t) { return foldMap_(id, t); };

constexpr auto foldl =
  curry([](const auto &f, const auto &acc, const auto &t) {
      return appEndo(getDual(foldMap_(
        [g = flip(f)](const auto &a) {
            return Dual(Endo<remove_cvref_t<decltype(acc)>>(g(a)));
        },
        t)))(acc);
  });

constexpr auto length = [](const auto &t) {
    return getSum(foldMap(konst(Sum(1)), t));
};

constexpr auto null = [](const auto &t) { return length(t) == 0; };

// O(n^2)
constexpr auto toVector = [](const auto &t) {
    return foldMap([](const auto &x) { return vector(1, x); }, t);
};

constexpr auto sum = [](const auto &t) {
    return getSum(foldMap(makeSum, t));
};

constexpr auto product = [](const auto &t) {
    return getProduct(foldMap(makeProduct, t));
};

constexpr auto concatMap = foldMap;

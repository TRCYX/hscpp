#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/operator_identifiers/M.inl>

template <typename M>
struct Monoid;

template <typename M>
constexpr bool is_monoid =
  is_same_v<remove_reference_t<decltype(Monoid<M>::memptyImpl)>, const M>
    &&is_same_v<remove_cvref_t<decltype(
                  Monoid<M>::mappendImpl(declval<M>(), declval<M>()))>,
                M>;

template <typename M>
inline const enable_if_t<is_monoid<M>, M> mempty = Monoid<M>::memptyImpl;

template <typename M>
inline constexpr M mappend_(const M &a, const M &b) {
    static_assert(is_monoid<M>);
    return Monoid<M>::mappendImpl(a, b);
}

constexpr auto mappend =
  curry([](const auto &a, const auto &b) { return mappend_(a, b); });

DECLARE_BINARY_OP(mappend, <M>);

template <typename M>
inline constexpr M mconcat_(const vector<M> &ms) {
    return accumulate(ms.begin(), ms.end(), mempty<M>, mappend_<M>);
}

constexpr auto mconcat = [](const auto &ms) { return mconcat_(ms); };

template <typename... Args>
struct Monoid<tuple<Args...>> {
    static_assert((is_monoid<Args> && ...));

    using type = tuple<Args...>;

    static constexpr type memptyImpl = tuple{mempty<Args>...};

    static constexpr type mappendImpl(const type &a, const type &b) {
        return mappendImplHelper(a, b, index_sequence_for<Args...>{});
    }

  private:
    template <size_t... I>
    static constexpr type mappendImplHelper(const type &a, const type &b,
                                            index_sequence<I...>) {
        return type{(get<I>(a) <M> get<I>(b))...};
    }
};

NEW_TYPE(All, getAll, bool);

template <>
struct Monoid<All> {
    static constexpr All memptyImpl = true;

    static constexpr All mappendImpl(All a, All b) {
        return getAll(a) && getAll(b);
    }
};

NEW_TYPE(Any, getAny, bool);

template <>
struct Monoid<Any> {
    static constexpr Any memptyImpl = false;

    static constexpr Any mappendImpl(Any a, Any b) {
        return getAny(a) && getAny(b);
    }
};

NEW_TYPE(Xor, getXor, bool);

template <>
struct Monoid<Xor> {
    static constexpr Xor memptyImpl = false;

    static constexpr Xor mappendImpl(Xor a, Xor b) {
        return getXor(a) ^ getXor(b);
    }
};

NEW_TYPE(Eqv, getEqv, bool);

template <>
struct Monoid<Eqv> {
    static constexpr Eqv memptyImpl = true;

    static constexpr Eqv mappendImpl(Eqv a, Eqv b) {
        return !(getEqv(a) ^ getEqv(b));
    }
};

NEW_ORDER_2_TYPE(Sum, getSum, T, is_arithmetic_v<T>, T);

template <typename T>
struct Monoid<Sum<T>> {
    static constexpr Sum<T> memptyImpl = 0;

    static constexpr Sum<T> mappendImpl(Sum<T> a, Sum<T> b) {
        return Sum<T>(getSum(a) + getSum(b));
    }
};

NEW_ORDER_2_TYPE(Product, getProduct, T, is_arithmetic_v<T>, T);

template <typename T>
struct Monoid<Product<T>> {
    static constexpr Product<T> memptyImpl = 1;

    static constexpr Product<T> mappendImpl(Product<T> a, Product<T> b) {
        return Product<T>(getProduct(a) * getProduct(b));
    }
};

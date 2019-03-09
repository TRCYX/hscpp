#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/typeclass/monoid.hpp>

NEW_ORDER_2_TYPE(Dual, getDual, A, true, A);

template <typename A>
struct Monoid<Dual<A>> {
    static_assert(is_monoid<A>);

    static inline const Dual<A> memptyImpl = Dual(mempty<A>);

    static constexpr Dual<A> mappendImpl(const Dual<A> &a, const Dual<A> &b) {
        return Dual(getDual(b) <M> getDual(a));
    }
};

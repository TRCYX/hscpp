#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/typeclass/monoid.hpp>

NEW_ORDER_2_TYPE(Endo, appEndo, T, true, function<T(T)>);

template <typename T>
struct Monoid<Endo<T>> {
    static inline const Endo<T> memptyImpl = Endo<T>(id);

    static constexpr Endo<T> mappendImpl(const Endo<T> &a, const Endo<T> &b) {
        return Endo<T>(compose(appEndo(a), appEndo(b)));
    }
};

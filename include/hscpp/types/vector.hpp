#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/typeclass/applicative.hpp>
#include <hscpp/typeclass/foldable.hpp>
#include <hscpp/typeclass/functor.hpp>
#include <hscpp/typeclass/monad.hpp>
#include <hscpp/typeclass/monoid.hpp>
#include <hscpp/typeclass/show.hpp>

template <typename A>
struct Show<vector<A>> {
    static_assert(showable<A>);

    static vector<char> showImpl(const vector<A> &v) {
        if (v.empty()) return "[]"_vs;

        ostringstream result;
        result << '[' << toStringView(show(v[0]));
        for (size_t i = 1; i < v.size(); ++i) {
            result << ", " << toStringView(show(v[i]));
        }
        result << ']';
        return toVs(result.str());
    }
};

template <>
struct Show<vector<char>> {
    static vector<char> showImpl(const vector<char> &v) {
        ostringstream result;
        result << '\"';
        for (size_t i = 0; i < v.size(); ++i) {
            if (v[i] == '\n') {
                result << '\\' << 'n';
            } else if (v[i] == '\t') {
                result << '\\' << 't';
            } else if (v[i] == '\r') {
                result << '\\' << 'r';
            } else if (v[i] == '\v') {
                result << '\\' << 'v';
            } else if (v[i] == '\a') {
                result << '\\' << 'a';
            } else if (v[i] == '\0') {
                result << '\\' << '0';
            } else if (v[i] == '\b') {
                result << '\\' << 'b';
            } else if (v[i] == '\e') {
                result << '\\' << 'e';
            } else if (v[i] == '\f') {
                result << '\\' << 'f';
            } else {
                if (v[i] == '\\' || v[i] == '\"') {
                    result << '\\';
                }
                result << v[i];
            }
        }
        result << '\"';

        return toVs(result.str());
    }
};

template <typename A>
struct Monoid<vector<A>> {
    static inline const vector<A> memptyImpl;

    static vector<A> mappendImpl(const vector<A> &a, const vector<A> &b) {
        return a + b;
    }
};

template <>
struct Functor<vector> {
    template <typename F, typename A, typename B = invoke_result_t<F, A>>
    static vector<B> fmapImpl(const F &f, const vector<A> &a) {
        vector<B> result;
        result.reserve(a.size());
        transform(a.begin(), a.end(), back_inserter(result), f);
        return result;
    }
};

template <>
struct Applicative<vector> {
    template <typename A>
    static vector<A> pureImpl(const A &a) {
        return {a};
    }

    template <typename F, typename A, typename B = invoke_result_t<F, A>>
    static vector<B> apImpl(const vector<F> &f, const vector<A> &a) {
        vector<B> result;
        result.reserve(f.size() * a.size());
        for_each(f.begin(), f.end(), [&a, &result](const F &f) {
            transform(a.begin(), a.end(), back_inserter(result), f);
        });
        return result;
    }
};

template <>
struct Monad<vector> {
    template <typename F, typename A>
    static auto bindMImpl(const F &f, const vector<A> &a) {
        vector<get_param_t<vector, invoke_result_t<F, A>>> result;
        for_each(a.begin(), a.end(), [&f, &result](const auto &a) {
            auto r = f(a);
            result.insert(result.end(), move_iterator(r.begin()),
                          move_iterator(r.end()));
        });
        return result;
    }
};

template <>
struct Foldable<vector> {
    template <typename F, typename B, typename A>
    static B foldrImpl(const F &f, const B &acc, const vector<A> &t) {
        return accumulate(t.rbegin(), t.rend(), acc, flip(f));
    }
};

template <>
struct Traversable<vector> {
    template <typename Func, typename A,
              typename F_B = remove_cvref_t<invoke_result_t<Func, A>>,
              ORDER_2_TYPE F =
                constructor_param_separator<F_B>::template constructor_t,
              typename B = typename constructor_param_separator<F_B>::param_t>
    static F<vector<B>> sequenceImpl(const Func &f, const vector<A> &t) {
        return foldl(
          [&f](const auto &acc, const auto &x) {
              return curry([](auto v, const auto &x) {
                  v.push_back(x);
                  return v;
              }) <$> acc <Ap> f(x);
          },
          F<vector<A>>(pure(vector<A>{})), t);
    }

    template <ORDER_2_TYPE F, typename A>
    static F<vector<A>> sequenceImpl(const vector<F<A>> &t) {
        return foldl(
          [](const auto &acc, const auto &x) {
              return curry([](auto v, const auto &x) {
                  v.push_back(x);
                  return v;
              }) <$> acc <Ap> x;
          },
          F<vector<A>>(pure(vector<A>{})), t);
    }
};

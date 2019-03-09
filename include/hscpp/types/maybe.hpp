#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/typeclass/applicative.hpp>
#include <hscpp/typeclass/foldable.hpp>
#include <hscpp/typeclass/functor.hpp>
#include <hscpp/typeclass/monad.hpp>
#include <hscpp/typeclass/monoid.hpp>
#include <hscpp/typeclass/show.hpp>
#include <hscpp/typeclass/traversable.hpp>

struct Nothing_t {
    explicit constexpr Nothing_t(int) {}
};

template <typename A>
class Maybe {
  public:
    constexpr explicit Maybe(optional<A> value) : content(move(value)) {}
    constexpr Maybe(A value) : content(move(value)) {}
    constexpr Maybe(Nothing_t) : content() {}
    constexpr Maybe() : content() {}

    constexpr const optional<A> &operator->() const { return content; }
    constexpr optional<A> &operator->() { return content; }
    constexpr const A &operator*() const & { return *content; }
    constexpr A &operator*() & { return *content; }
    constexpr const A &&operator*() const && { return *content; }
    constexpr A &&operator*() && { return *content; }

    constexpr bool has_value() const noexcept { return bool(content); }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    constexpr const A &value() const & { return content.value(); }
    constexpr A &value() & { return content.value(); }
    constexpr const A &&value() const && { return content.value(); }
    constexpr A &&value() && { return content.value(); }

    template <typename B>
    constexpr A value_or(B &&default_value) const & {
        return content.value_or(forward<B>(default_value));
    }
    template <typename B>
    constexpr A value_or(B &&default_value) && {
        return content.value_or(forward<B>(default_value));
    }

  private:
    optional<A> content;

    friend std::hash<Maybe<A>>;
};

template <typename A>
struct std::hash<Maybe<A>> {
    constexpr size_t operator()(const Maybe<A> &s) const {
        return hash<optional<A>>{}(s.content);
    }
};

constexpr auto Just = [](auto value) {
    return Maybe<decltype(value)>(move(value));
};

constexpr Nothing_t Nothing{0};

template <typename A>
struct Show<Maybe<A>> {
    static_assert(showable<A>);

    static vector<char> showImpl(const Maybe<A> &o) {
        return o ? "Just "_vs + show(*o) : "Nothing"_vs;
    }
};

template <typename A>
struct Monoid<Maybe<A>> {
    static_assert(is_monoid<A>);

    static inline const Maybe<A> memptyImpl;

    constexpr static Maybe<A> mappendImpl(const Maybe<A> &a, const Maybe<A> &b) {
        return a ? (b ? Just(*a <M> *b) : a) : b;
    }
};

template <>
struct Foldable<Maybe> {
    template <typename F, typename A, typename M = invoke_result_t<F, A>>
    constexpr static M foldMapImpl(const F &f, const Maybe<A> &a) {
        return a ? f(*a) : mempty<M>;
    }

    template <typename F, typename A, typename B>
    constexpr static B foldrImpl(const F &f, const B &acc, const Maybe<A> &a) {
        return a ? f(*a, acc) : acc;
    }
};

template <>
struct Functor<Maybe> {
    template <typename F, typename A, typename B = invoke_result_t<F, A>>
    constexpr static Maybe<B> fmapImpl(const F &f, const Maybe<A> &a) {
        return a ? Just(f(*a)) : Nothing;
    }
};

template <>
struct Applicative<Maybe> {
    template <typename A>
    constexpr static Maybe<A> pureImpl(const A &a) {
        return a;
    }

    template <typename F, typename A, typename B = invoke_result_t<F, A>>
    constexpr static Maybe<B> apImpl(const Maybe<F> &f, const Maybe<A> &a) {
        return f && a ? Just((*f)(*a)) : Nothing;
    }
};

template <>
struct Monad<Maybe> {
    template <typename F, typename A>
    constexpr static auto bindMImpl(const F &f, const Maybe<A> &a) {
        return a ? f(*a) : Nothing;
    }
};

template <>
struct Traversable<Maybe> {
    template <ORDER_2_TYPE F, typename A>
    constexpr static F<Maybe<A>> sequenceImpl(const Maybe<F<A>> &t) {
        return t ? Just <$> *t : pure(Nothing);
    }
};

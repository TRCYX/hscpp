#pragma once

#include <hscpp/basics.hpp>
#include <hscpp/typeclass/applicative.hpp>
#include <hscpp/typeclass/functor.hpp>
#include <hscpp/typeclass/monad.hpp>
#include <hscpp/typeclass/monoid.hpp>
#include <hscpp/typeclass/show.hpp>
#include <hscpp/typeclass/traversable.hpp>
#include <hscpp/types/vector.hpp>

template <typename T>
class IO;

template <typename T>
T runIO(const IO<T> &io);

template <typename T>
struct IOPrimitive {
    const function<T()> run;
};

template <typename T>
struct IOPure {
    explicit IOPure(T t) : value(move(t)) {}

    const T value;

    T run() const { return value; }
};

template <typename T>
struct IOBind {
    template <
      typename F, typename A,
      typename = enable_if_t<
        is_same_v<remove_cvref_t<invoke_result_t<F, A>>, IO<T>>, void>>
    IOBind(F f, IO<A> a)
        : content(make_shared<IOBind<T>::Content<A>>(move(f), move(a))) {}

    T run() const { return content->run(); }

    struct ContentBase {
        virtual T run() const = 0;
        virtual ~ContentBase() = default;
    };

    template <typename A>
    struct Content : ContentBase {
        template <typename F, typename A_>
        Content(F &&f, A_ &&a) : f(forward<F>(f)), a(forward<A_>(a)) {}

        T run() const override {
            // The order of execution is created here.
            A a_result = runIO(a);
            return runIO(f(a_result));
        }

        const function<IO<T>(A)> f;
        const IO<A> a;
    };

    const shared_ptr<const ContentBase> content;
};

template <typename F, typename A>
IOBind(F, IO<A>)
  ->IOBind<get_param_t<IO, remove_cvref_t<invoke_result_t<F, A>>>>;

template <typename T>
struct IO {
    IO(IOPrimitive<T> t) : content(move(t)) {}
    IO(IOPure<T> t) : content(move(t)) {}
    IO(IOBind<T> t) : content(move(t)) {}

    variant<IOPrimitive<T>, IOPure<T>, IOBind<T>> content;
};

template <typename T>
T runIO(const IO<T> &io) {
    return visit([](const auto &c) { return c.run(); }, io.content);
}

template <>
struct Functor<IO> {
    template <typename F, typename A, typename B = invoke_result_t<F, A>>
    static IO<B> fmapImpl(const F &f, const IO<A> &a) {
        return IO<F>(pure(f)) <Ap> a;
    }
};

template <>
struct Applicative<IO> {
    template <typename A>
    static IO<A> pureImpl(const A &a) {
        return IOPure(a);
    }

    template <typename F, typename A, typename B = invoke_result_t<F, A>>
    static IO<B> apImpl(const IO<F> &f, const IO<A> &a) {
        return f >> [a](const F &f) {
            return a >> [f](const A &a) -> IO<B> { return pure(f(a)); };
        };
    }
};

template <>
struct Monad<IO> {
    template <typename F, typename A>
    static auto bindMImpl(const F &f, const IO<A> &a) {
        return IO(IOBind(f, a));
    }
};

template <typename A>
struct Monoid<IO<A>> {
    static_assert(is_monoid<A>);

    static inline const IO<A> memptyImpl = pure(mempty<A>);

    static IO<A> mappendImpl(const IO<A> &a, const IO<A> &b) {
        return mappend <$> a <Ap> b;
    }
};

inline const IO<char> getChar = IOPrimitive<char>{[]() { return cin.get(); }};

constexpr auto putChar = [](char c) -> IO<tuple<>> {
    return IOPrimitive<tuple<>>{[c]() {
        cout << c;
        return tuple<>{};
    }};
};

IO<vector<char>> getLineWith(vector<char> prefix) {
    return getChar >>
             [prefix = move(prefix)](char c) mutable -> IO<vector<char>> {
        if (c == '\n') {
            return pure(prefix);
        } else {
            prefix.push_back(c);
            return getLineWith(move(prefix));
        }
    };
}

inline const IO<vector<char>> getLine = getLineWith({});

inline const auto putStr = traverse(putChar);

constexpr auto putStrLn = [](const vector<char> &vs) {
    return putStr(vs) *Ap> putChar('\n');
};

constexpr auto print = compose(putStrLn, show);

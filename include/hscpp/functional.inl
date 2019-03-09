#pragma once

#include <hscpp/includes.inl>

template <typename F, typename... Xs>
struct currying_helper {
    template <typename... Ys>
    using type = invoke_result_t<F, Xs..., Ys...>;
};

template <typename F>
constexpr auto curry_(F f) {
    if constexpr (is_invocable_v<F>) {
        return f();
    } else {
        return [f = move(f)](auto... xs) {
            using currying_helper =
              currying_helper<decltype(f), decltype(move(xs))...>;
            return curry_(
              [ f = move(f), xs = make_tuple(move(xs)...) ](auto... ys) ->
              typename currying_helper::template type<decltype(move(ys))...> {
                  return apply(f, tuple_cat(xs, make_tuple(move(ys)...)));
              });
        };
    }
}

constexpr auto curry = [](auto f) { return curry_(move(f)); };

constexpr auto id = [](auto x) { return x; };

constexpr auto konst = [](auto x) {
    return [x = move(x)](auto y) { return x; };
};

constexpr auto seq = konst(id);

constexpr auto flip = [](auto f) {
    return curry([f = move(f)](auto x, auto y) { return f(y, x); });
};

constexpr auto compose = curry([](auto f, auto g) {
    return [f = move(f), g = move(g)](auto x) { return f(g(x)); };
});

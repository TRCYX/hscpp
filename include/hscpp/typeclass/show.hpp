#pragma once

#include <hscpp/basics.hpp>

template <typename S, typename = void>
struct Show;

template <typename S>
inline constexpr bool showable =
  is_same_v<vector<char>, decltype(Show<S>::showImpl(declval<S>()))>;

template <typename S>
inline vector<char> show_(const S &s) {
    static_assert(showable<S>);
    return Show<S>::showImpl(s);
}

constexpr auto show = [](const auto &s) { return show_(s); };

template <>
struct Show<char> {
    static vector<char> showImpl(char x) { return vector{'\'', x, '\''}; };
};

template <typename T>
struct Show<T, void_t<decltype(to_string(declval<T>()))>> {
    static vector<char> showImpl(T x) { return toVs(to_string(x)); }
};

template <typename... Args>
struct Show<tuple<Args...>> {
    static_assert((showable<Args> && ...));

    using type = tuple<Args...>;

    static vector<char> showImpl(const type &x) {
        if constexpr (sizeof...(Args) == 0) {
            return "()"_vs;
        } else {
            return showImplHelper(x, index_sequence_for<Args...>{});
        }
    }

  private:
    template <size_t I, size_t... Is>
    static vector<char> showImplHelper(const type &x, index_sequence<I, Is...>) {
        ostringstream result;
        result << '(' << toStringView(show(get<I>(x)));
        ((result << ", " << toStringView(show(get<Is>(x)))), ...);
        result << ')';
        return toVs(result.str());
    }
};

template <typename S>
struct Show<
  S, enable_if_t<is_new_type_v<S> && showable<typename S::base_type>, void>> {
    static vector<char> showImpl(const S &s) {
        return toVs(S::type_name) + " {"_vs + toVs(S::getter_name) +
               " = "_vs + show(s.value) + '}';
    }
};

#pragma once

#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

using namespace std;
using namespace placeholders;

template <typename T>
vector<T> toVs(const basic_string<T> &s) {
    return vector<T>(s.begin(), s.end());
}

template <typename T>
vector<T> toVs(const basic_string_view<T> &s) {
    return vector<T>(s.begin(), s.end());
}

template <typename T>
basic_string<T> toString(const vector<T> &vs) {
    basic_string<T> s;
    s.reserve(vs.size());
    copy(vs.begin(), vs.end(), back_inserter(s));
    return s;
}

template <typename T>
basic_string_view<T> toStringView(const vector<T> &vs) {
    return basic_string_view<T>(vs.data(), vs.size());
}

vector<char> operator""_vs(const char *str, size_t len) {
    return vector<char>(str, str + len);
}

template <typename T>
vector<T> operator+(vector<T> lhs, const vector<T> &rhs) {
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    return lhs;
}

template <typename T>
vector<T> operator+(vector<T> lhs, T rhs) {
    lhs.push_back(move(rhs));
    return lhs;
}

template <typename T>
vector<T> operator+(T lhs, vector<T> rhs) {
    rhs.insert(rhs.begin(), move(lhs));
    return rhs;
}

template <typename T>
basic_string<T> operator+(const basic_string_view<T> &lhs,
                          basic_string<T> rhs) {
    return string(lhs) + move(rhs);
}

template <typename T>
basic_string<T> operator+(basic_string<T> lhs,
                          const basic_string_view<T> &rhs) {
    return move(lhs) + string(rhs);
}

template <typename T>
basic_string<T> operator+(const basic_string_view<T> &lhs,
                          const basic_string_view<T> &rhs) {
    return string(lhs) + string(rhs);
}

template <typename F>
struct Validator {
    constexpr Validator(F f) : f(move(f)) {}

    template <typename... Args>
    static constexpr bool is_valid = is_invocable_v<F, Args...>;

    F f;
};

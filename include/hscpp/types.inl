#pragma once

#include <hscpp/includes.inl>

#define ORDER_2_TYPE                                                         \
    template <typename>                                                      \
    typename
#define ORDER_3_TYPE                                                         \
    template <ORDER_2_TYPE>                                                  \
    typename
#define ORDER_4_TYPE                                                         \
    template <ORDER_3_TYPE>                                                  \
    typename

template <ORDER_2_TYPE T, typename A>
struct get_param;

template <ORDER_2_TYPE T, typename A>
struct get_param<T, T<A>> {
    using type = A;
};

template <ORDER_2_TYPE T, typename A>
using get_param_t = typename get_param<T, remove_cv_t<A>>::type;

template <typename T>
struct constructor_param_separator;

template <ORDER_2_TYPE F, typename A>
struct constructor_param_separator<F<A>> {
    template <typename T>
    using constructor_t = F<T>;
    using param_t = A;
};

template <typename T>
using remove_cvref_t = remove_cv_t<remove_reference_t<T>>;

template <typename T>
struct id_helper;

template <typename T>
struct id_helper<void(T)> {
    using type = T;
};

template <typename T>
using id_helper_t = typename id_helper<T>::type;

#define ID(...) id_helper_t<void(common_type_t<__VA_ARGS__>)>

template <typename T>
struct is_new_type : false_type {};
template <typename T>
constexpr bool is_new_type_v = is_new_type<T>::value;

#define NEW_TYPE(name, getter, type)                                         \
    struct name {                                                            \
        using base_type = common_type_t<type>;                               \
        static constexpr string_view type_name = #name;                      \
        static constexpr string_view getter_name = #getter;                  \
                                                                             \
        constexpr name(base_type x) : value(move(x)) {}                      \
                                                                             \
        base_type value;                                                     \
    };                                                                       \
                                                                             \
    constexpr name make##name(name::base_type x) { return name(move(x)); }   \
                                                                             \
    constexpr name::base_type &getter##_(name &x) { return x.value; }        \
    constexpr const name::base_type &getter##_(const name &x) {              \
        return x.value;                                                      \
    }                                                                        \
    constexpr auto getter = [](auto &&x) -> decltype(auto) {                 \
        return getter##_(forward<decltype(x)>(x));                           \
    };                                                                       \
                                                                             \
    template <>                                                              \
    struct is_new_type<name> : true_type {};

#define NEW_ORDER_2_TYPE(name, getter, template_arg, assertion, type)        \
    template <typename template_arg>                                         \
    struct name {                                                            \
        static_assert(assertion);                                            \
                                                                             \
        using base_type = common_type_t<type>;                               \
        static constexpr string_view type_name = #name;                      \
        static constexpr string_view getter_name = #getter;                  \
                                                                             \
        constexpr name(base_type x) : value(move(x)) {}                      \
                                                                             \
        base_type value;                                                     \
    };                                                                       \
                                                                             \
    template <typename template_arg>                                         \
    name(type)->name<template_arg>;                                          \
                                                                             \
    constexpr auto make##name = [](auto &&x) {                               \
        return name(forward<decltype(x)>(x));                                \
    };                                                                       \
                                                                             \
    template <typename template_arg>                                         \
    constexpr typename name<template_arg>::base_type &getter##_(             \
      name<template_arg> &x) {                                               \
        return x.value;                                                      \
    }                                                                        \
    template <typename template_arg>                                         \
    constexpr const typename name<template_arg>::base_type &getter##_(       \
      const name<template_arg> &x) {                                         \
        return x.value;                                                      \
    }                                                                        \
    constexpr auto getter = [](auto &&x) -> decltype(auto) {                 \
        return getter##_(forward<decltype(x)>(x));                           \
    };                                                                       \
                                                                             \
    template <typename template_arg>                                         \
    struct is_new_type<name<template_arg>> : true_type {};

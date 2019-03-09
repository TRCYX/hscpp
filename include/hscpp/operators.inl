#pragma once

#include <hscpp/functional.inl>
#include <hscpp/includes.inl>
#include <hscpp/types.inl>

#define PREFIX_OP(op) ([](const auto &x) { return op x; })
#define SUFFIX_OP(op) ([](const auto &x) { return x op; })
#define BINARY_OP(op)                                                        \
    (curry([](const auto &x, const auto &y) { return x op y; }))

namespace _op {

struct no_such_operator {};

template <typename Expr>
constexpr no_such_operator bin_op;

enum class OpCode;

template <typename T, typename = void>
struct content {
    using type = T;
};

template <typename T>
using content_t = typename content<T>::type;

template <typename T>
constexpr decltype(auto) content_of(T &&t) {
    return forward<T>(t);
}

struct no_operand_t {};

constexpr no_operand_t no_operand;

template <typename T, typename = void>
struct expr {
    using type = no_operand_t;
};

template <typename T>
using expr_t = typename expr<T>::type;

template <typename Lhs, OpCode op_code, typename Rhs>
struct infix_expr {
    static_assert(!is_reference_v<Lhs> && !is_reference_v<Rhs>);
    static_assert(!is_const_v<Lhs> && !is_const_v<Rhs>);
};

template <typename T>
constexpr bool is_infix_expr = false;

template <typename T>
struct content<T, enable_if_t<is_infix_expr<T>>> {
    // no type here
};

template <typename T>
struct expr<T, enable_if_t<is_infix_expr<T>>> {
    using type = remove_cvref_t<T>;
};

template <typename T>
constexpr bool is_infix_expr<T &> = is_infix_expr<T>;
template <typename T>
constexpr bool is_infix_expr<T &&> = is_infix_expr<T>;
template <typename T>
constexpr bool is_infix_expr<const T> = is_infix_expr<T>;
template <>
constexpr bool is_infix_expr<no_operand_t> = true;
template <typename Lhs, OpCode op_code, typename Rhs>
constexpr bool is_infix_expr<infix_expr<Lhs, op_code, Rhs>> = true;

template <typename T, typename Expr>
struct prefix_expr {
    static_assert(is_infix_expr<Expr>);

    T content;
};

template <typename T, typename Expr>
struct content<prefix_expr<T, Expr>> {
    using type = T;
};

template <typename T, typename Expr>
constexpr decltype(auto) content_of(prefix_expr<T, Expr> p) {
    return forward<T>(p.content);
}

template <typename T, typename Expr>
struct expr<prefix_expr<T, Expr>> {
    using type = Expr;
};

template <typename T>
constexpr bool is_prefix_expr = false;
template <typename T>
constexpr bool is_prefix_expr<T &> = is_prefix_expr<T>;
template <typename T>
constexpr bool is_prefix_expr<T &&> = is_prefix_expr<T>;
template <typename T>
constexpr bool is_prefix_expr<const T> = is_prefix_expr<T>;
template <typename T, typename Expr>
constexpr bool is_prefix_expr<prefix_expr<T, Expr>> = true;

template <typename T, typename Expr>
struct suffix_expr {
    static_assert(is_infix_expr<Expr>);

    T content;
};

template <typename T, typename Expr>
struct content<suffix_expr<T, Expr>> {
    using type = T;
};

template <typename T, typename Expr>
constexpr decltype(auto) content_of(suffix_expr<T, Expr> s) {
    return forward<T>(s.content);
}

template <typename T, typename Expr>
struct expr<suffix_expr<T, Expr>> {
    using type = Expr;
};

template <typename T>
constexpr bool is_suffix_expr = false;
template <typename T>
constexpr bool is_suffix_expr<T &> = is_suffix_expr<T>;
template <typename T>
constexpr bool is_suffix_expr<T &&> = is_suffix_expr<T>;
template <typename T>
constexpr bool is_suffix_expr<const T> = is_suffix_expr<T>;
template <typename T, typename Expr>
constexpr bool is_suffix_expr<suffix_expr<T, Expr>> = true;

template <typename T>
constexpr bool is_not_expr =
  !(is_infix_expr<T> || is_prefix_expr<T> || is_suffix_expr<T>);
template <typename T>
constexpr bool is_not_expr<T &> = is_not_expr<T>;
template <typename T>
constexpr bool is_not_expr<T &&> = is_not_expr<T>;
template <typename T>
constexpr bool is_not_expr<const T> = is_not_expr<T>;

template <typename T>
constexpr bool can_be_prefix = is_prefix_expr<T> || is_not_expr<T>;
template <typename T>
constexpr bool can_be_suffix = is_suffix_expr<T> || is_not_expr<T>;

template <typename Lhs, typename Rhs>
constexpr bool can_produce_infix = is_infix_expr<Lhs> &&is_infix_expr<Rhs>;
template <typename Lhs, typename Rhs>
constexpr bool can_produce_prefix = can_be_prefix<Lhs> &&is_infix_expr<Rhs>;
template <typename Lhs, typename Rhs>
constexpr bool can_produce_suffix = is_infix_expr<Lhs> &&can_be_suffix<Rhs>;
template <typename Lhs, typename Rhs>
constexpr bool can_produce_full = can_be_prefix<Lhs> &&can_be_suffix<Rhs> &&
                                  !(is_not_expr<Lhs> && is_not_expr<Rhs>);

template <typename Rhs>
constexpr bool can_produce_prefix_intermediate =
  is_infix_expr<Rhs> || is_suffix_expr<Rhs>;
template <typename Lhs>
constexpr bool can_produce_suffix_intermediate =
  is_infix_expr<Lhs> || is_prefix_expr<Lhs>;
template <typename Lhs, typename Rhs>
constexpr bool can_produce_binary_intermediate =
  can_produce_infix<Lhs, Rhs> || can_produce_prefix<Lhs, Rhs> ||
  can_produce_suffix<Lhs, Rhs> || can_produce_full<Lhs, Rhs>;

#define DECLARE_SUFFIX_INTERMEDIATE_OP(op, code)                             \
    template <typename Lhs,                                                  \
              typename = enable_if_t<can_produce_suffix_intermediate<Lhs>>>  \
    constexpr auto operator op(Lhs &&lhs) {                                  \
        using Expr = infix_expr<expr_t<Lhs>, code, no_operand_t>;            \
                                                                             \
        if constexpr (is_infix_expr<Lhs>) {                                  \
            return Expr{};                                                   \
        } else if constexpr (is_prefix_expr<Lhs>) {                          \
            return prefix_expr<content_t<Lhs> &&, Expr>{                     \
              content_of(forward<Lhs>(lhs))};                                \
        }                                                                    \
    }

#define DECLARE_PREFIX_INTERMEDIATE_OP(op, code)                             \
    template <typename Rhs,                                                  \
              typename = enable_if_t<can_produce_prefix_intermediate<Rhs>>>  \
    constexpr auto operator op(Rhs &&rhs) {                                  \
        using Expr = infix_expr<no_operand_t, code, expr_t<Rhs>>;            \
                                                                             \
        if constexpr (is_infix_expr<Rhs>) {                                  \
            return Expr{};                                                   \
        } else if constexpr (is_suffix_expr<Rhs>) {                          \
            return suffix_expr<content_t<Rhs> &&, Expr>{                     \
              content_of(forward<Rhs>(rhs))};                                \
        }                                                                    \
    }

#define DECLARE_BINARY_INTERMEDIATE_OP(op, code)                             \
    template <typename Lhs, typename Rhs,                                    \
              typename =                                                     \
                enable_if_t<can_produce_binary_intermediate<Lhs, Rhs>>>      \
    constexpr auto operator op(Lhs &&lhs, Rhs &&rhs) {                       \
        using Expr = infix_expr<expr_t<Lhs>, code, expr_t<Rhs>>;             \
                                                                             \
        if constexpr (can_produce_infix<Lhs, Rhs>) {                         \
            return Expr{};                                                   \
        } else if constexpr (can_produce_prefix<Lhs, Rhs>) {                 \
            return prefix_expr<content_t<Lhs> &&, Expr>{                     \
              content_of(forward<Lhs>(lhs))};                                \
        } else if constexpr (can_produce_suffix<Lhs, Rhs>) {                 \
            return suffix_expr<content_t<Rhs> &&, Expr>{                     \
              content_of(forward<Rhs>(rhs))};                                \
        } else if constexpr (can_produce_full<Lhs, Rhs>) {                   \
            return bin_op<Expr>(content_of(forward<Lhs>(lhs)),               \
                                content_of(forward<Rhs>(rhs)));              \
        }                                                                    \
    }

enum class OpCode {
    PostInc,
    PostDec,
    PreInc,
    PreDec,
    Pos,
    Neg,
    Compl,
    Mul,
    Div,
    Mod,
    Plus,
    Minus,
    Lsh,
    Rsh,
    LT,
    GT,
    LE,
    GE,
    EQ,
    NE,
    BitAnd,
    BitXor,
    BitOr,
    Comma
};

template <typename Lhs,
          typename = enable_if_t<can_produce_suffix_intermediate<Lhs>>>
constexpr auto operator++(Lhs &&lhs, int) {
    using Expr = infix_expr<expr_t<Lhs>, OpCode::PostInc, no_operand_t>;

    if constexpr (is_infix_expr<Lhs>) {
        return Expr{};
    } else if constexpr (is_prefix_expr<Lhs>) {
        return prefix_expr<content_t<Lhs> &&, Expr>{
          content_of(forward<Lhs>(lhs))};
    }
}

template <typename Lhs,
          typename = enable_if_t<can_produce_suffix_intermediate<Lhs>>>
constexpr auto operator--(Lhs &&lhs, int) {
    using Expr = infix_expr<expr_t<Lhs>, OpCode::PostDec, no_operand_t>;

    if constexpr (is_infix_expr<Lhs>) {
        return Expr{};
    } else if constexpr (is_prefix_expr<Lhs>) {
        return prefix_expr<content_t<Lhs> &&, Expr>{
          content_of(forward<Lhs>(lhs))};
    }
}

DECLARE_PREFIX_INTERMEDIATE_OP(++, OpCode::PreInc);
DECLARE_PREFIX_INTERMEDIATE_OP(--, OpCode::PreDec);
DECLARE_PREFIX_INTERMEDIATE_OP(+, OpCode::Pos);
DECLARE_PREFIX_INTERMEDIATE_OP(-, OpCode::Neg);
DECLARE_PREFIX_INTERMEDIATE_OP(~, OpCode::Compl);

DECLARE_BINARY_INTERMEDIATE_OP(*, OpCode::Mul);
DECLARE_BINARY_INTERMEDIATE_OP(/, OpCode::Div);
DECLARE_BINARY_INTERMEDIATE_OP(%, OpCode::Mod);
DECLARE_BINARY_INTERMEDIATE_OP(+, OpCode::Plus);
DECLARE_BINARY_INTERMEDIATE_OP(-, OpCode::Minus);
DECLARE_BINARY_INTERMEDIATE_OP(<<, OpCode::Lsh);
DECLARE_BINARY_INTERMEDIATE_OP(>>, OpCode::Rsh);
DECLARE_BINARY_INTERMEDIATE_OP(<, OpCode::LT);
DECLARE_BINARY_INTERMEDIATE_OP(>, OpCode::GT);
DECLARE_BINARY_INTERMEDIATE_OP(<=, OpCode::LE);
DECLARE_BINARY_INTERMEDIATE_OP(>=, OpCode::GE);
DECLARE_BINARY_INTERMEDIATE_OP(==, OpCode::EQ);
DECLARE_BINARY_INTERMEDIATE_OP(!=, OpCode::NE);
DECLARE_BINARY_INTERMEDIATE_OP(&, OpCode::BitAnd);
DECLARE_BINARY_INTERMEDIATE_OP(^, OpCode::BitXor);
DECLARE_BINARY_INTERMEDIATE_OP(|, OpCode::BitOr);

#define COMMA ,
DECLARE_BINARY_INTERMEDIATE_OP(COMMA, OpCode::Comma);
#undef COMMA

#undef DECLARE_PREFIX_INTERMEDIATE_OP
#undef DECLARE_SUFFIX_INTERMEDIATE_OP
#undef DECLARE_BINARY_INTERMEDIATE_OP

template <typename Identifier>
struct identifier_expr {};

template <typename Identifier>
constexpr bool is_infix_expr<identifier_expr<Identifier>> = true;

} // namespace _op

#define DECLARE_OP_IDENTIFIER(identifier)                                    \
    struct _op##identifier {};                                               \
    constexpr _op::identifier_expr<_op##identifier> identifier;

#define DECLARE_BINARY_OP(func, ...)                                         \
    namespace _op {                                                          \
    template <>                                                              \
    constexpr decltype(auto)                                                 \
      bin_op<expr_t<decltype(no_operand __VA_ARGS__ no_operand)>> = (func);  \
    }

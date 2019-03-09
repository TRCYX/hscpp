#include <hscpp/prelude.hpp>

int main() {
    Maybe<int> a = Just(2), b;
    vector<int> v{1, 2, 3}, u{10, 20};
    vector<vector<int>> vv{{1, 2}, {3, 4}, {5}};
    Maybe<vector<int>> av, bv = Just(vector{1, 1, 1});

    auto add = curry([](auto x, auto y) { return x + y; });
    auto add1 = add(1);
    auto step = [](auto x) {
        return x > 0 ? Just(100 - x * x) : Nothing;
    };
    auto replicate = [](auto x) { return vector((size_t)x, x); };

    runIO(IO<tuple<>>(pure(tuple{}))
        >M> putStrLn("abc"_vs)
        // >M> getChar
        // >M> mappend(getLine, getLine)

        >M> print(a)
        >M> print(v)
        >M> print(vv)
        >M> print('a')
        >M> print("abc"_vs + "def"_vs)
        >M> print(tuple{tuple{1}, 'a', "bc"_vs, tuple{}})

        >M> print(fmap(add1, a))
        >M> print(fmap(add1, v))
        >M> print(fmap(add1) <$> vv)

        >M> print(add <$> a <Ap> b)
        >M> print(add <$> u <Ap> v)
        >M> print(u *Ap> v)
        >M> print(u <Ap* v)

        >M> print(a >> (step >_> step))
        >M> print(v >> replicate >> replicate)
        >M> print(vv >> id)

        >M> print(mconcat(vv))
        >M> print(av <M> av)
        >M> print(av <M> bv)
        >M> print(bv <M> bv)
        >M> print(Sum(5) <M> Sum(6))
        >M> print(tuple{Sum(5), Product(2)} <M> tuple{Sum(4), Product(3)})
        >M> print(mappend(Sum(5), Sum(6)))

        >M> print(foldr(add, u, vv))
        >M> print(foldMap(makeProduct, v))
        >M> print(fold(vv))
        >M> print(foldl(add, u, vv))
        >M> print(length(u))
        >M> print(length(a))

        >M> print(sequence(av))
        >M> print(sequence(bv))
        >M> print(traverse(step, v))
        >M> print(traverse(replicate, a))
        >M> print(sequence(vv))
    );
}

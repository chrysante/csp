#define FASTVIS_IMPL_ENABLE_DEBUGGING

#include <functional>

#include <fastvis.hpp>

/// # Enum reflection tests

namespace {

enum class TestEnum { A, B, C };

namespace XY {

enum class TestEnumNegative { A = -3, B, C };

}

enum Unscoped { A, B, C };

template <auto V>
struct StrangeScope {
    enum E { A, B, C };
};

} // namespace

#define CHECK_NOTHROW(...)                                                     \
    do {                                                                       \
        try {                                                                  \
            __VA_ARGS__;                                                       \
        }                                                                      \
        catch (...) {                                                          \
            assert(false);                                                     \
        }                                                                      \
    } while (0)

#define CHECK_THROWS(...)                                                      \
    do {                                                                       \
        try {                                                                  \
            __VA_ARGS__;                                                       \
            assert(false);                                                     \
        }                                                                      \
        catch (...) {                                                          \
        }                                                                      \
    } while (0)

static_assert(!fastvis::impl::enumIsValid<TestEnum, -2>());
static_assert(!fastvis::impl::enumIsValid<TestEnum, -1>());
static_assert(fastvis::impl::enumIsValid<TestEnum, TestEnum::A>());
static_assert(fastvis::impl::enumIsValid<TestEnum, TestEnum::B>());
static_assert(fastvis::impl::enumIsValid<TestEnum, TestEnum::C>());
static_assert(!fastvis::impl::enumIsValid<TestEnum, 3>());
static_assert(!fastvis::impl::enumIsValid<TestEnum, 4>());

static_assert(fastvis::impl::enumRangeFirst<TestEnum>() == 0);
static_assert(fastvis::impl::enumRangeLast<TestEnum>() == 3);
static_assert(fastvis::impl::enumCount<TestEnum>() == 3);

static_assert(!fastvis::impl::enumIsValid<XY::TestEnumNegative, -5>());
static_assert(!fastvis::impl::enumIsValid<XY::TestEnumNegative, -4>());
static_assert(fastvis::impl::enumIsValid<XY::TestEnumNegative,
                                         XY::TestEnumNegative::A>());
static_assert(fastvis::impl::enumIsValid<XY::TestEnumNegative,
                                         XY::TestEnumNegative::B>());
static_assert(fastvis::impl::enumIsValid<XY::TestEnumNegative,
                                         XY::TestEnumNegative::C>());
static_assert(!fastvis::impl::enumIsValid<XY::TestEnumNegative, 0>());
static_assert(!fastvis::impl::enumIsValid<XY::TestEnumNegative, 1>());

static_assert(fastvis::impl::enumRangeFirst<XY::TestEnumNegative, -128>() ==
              -3);
static_assert(fastvis::impl::enumRangeLast<XY::TestEnumNegative, -128>() == 0);
static_assert(fastvis::impl::enumCount<XY::TestEnumNegative>() == 3);

static_assert(fastvis::impl::enumRangeFirst<Unscoped>() == 0);
static_assert(fastvis::impl::enumRangeLast<Unscoped>() == 3);
static_assert(fastvis::impl::enumCount<Unscoped>() == 3);

static_assert(
    fastvis::impl::enumRangeFirst<StrangeScope<[]<size_t I>() {}>::E>() == 0);
static_assert(
    fastvis::impl::enumRangeLast<StrangeScope<[]<size_t I>() {}>::E>() == 3);
static_assert(fastvis::impl::enumCount<StrangeScope<[]<size_t I>() {}>::E>() ==
              3);

/// # Index expansion tests

namespace fastvis::impl {

template <typename T, size_t N>
constexpr bool operator==(Array<T, N> const& A, Array<T, N> const& B) {
    for (size_t i = 0; i < N; ++i) {
        if (A[i] != B[i]) {
            return false;
        }
    }
    return true;
}

} // namespace fastvis::impl

static void testInternals() {
    using namespace fastvis::impl;
    // Single dimensional case
    static_assert(flattenIndex<1>({ 0 }, { 3 }) == 0);
    static_assert(expandIndex<1>(0, { 3 }) == Array<size_t, 1>{ 0 });
    static_assert(flattenIndex<1>({ 2 }, { 3 }) == 2);
    static_assert(expandIndex<1>(2, { 3 }) == Array<size_t, 1>{ 2 });

    // +---+---+
    // | 0 | 1 |
    // +---+---+
    // | 2 | 3 |
    // +---+---+
    static_assert(flattenIndex<2>({ 1, 1 }, { 2, 2 }) == 3);
    static_assert(expandIndex<2>(3, { 2, 2 }) == Array<size_t, 2>{ 1, 1 });

    // +---+---+---+
    // | 0 | 1 | 2 |
    // +---+---+---+
    // | 3 | 4 | 5 |
    // +---+---+---+
    static_assert(flattenIndex<2>({ 1, 1 }, { 2, 3 }) == 4);
    static_assert(expandIndex<2>(4, { 2, 3 }) == Array<size_t, 2>{ 1, 1 });
    static_assert(flattenIndex<2>({ 0, 2 }, { 2, 3 }) == 2);
    static_assert(expandIndex<2>(2, { 2, 3 }) == Array<size_t, 2>{ 0, 2 });

    // +---+---+
    // | 0 | 1 |
    // +---+---+
    // | 2 | 3 |
    // +---+---+
    // | 4 | 5 |
    // +---+---+
    static_assert(flattenIndex<2>({ 1, 1 }, { 3, 2 }) == 3);
    static_assert(expandIndex<2>(3, { 3, 2 }) == Array<size_t, 2>{ 1, 1 });
    static_assert(flattenIndex<2>({ 0, 1 }, { 3, 2 }) == 1);
    static_assert(expandIndex<2>(1, { 3, 2 }) == Array<size_t, 2>{ 0, 1 });
    static_assert(flattenIndex<2>({ 2, 0 }, { 3, 2 }) == 4);
    static_assert(expandIndex<2>(4, { 3, 2 }) == Array<size_t, 2>{ 2, 0 });
    static_assert(flattenIndex<2>({ 2, 1 }, { 3, 2 }) == 5);
    static_assert(expandIndex<2>(5, { 3, 2 }) == Array<size_t, 2>{ 2, 1 });

    // +---+---+
    // | 0 | 3 |---+---+
    // +---+---+ 1 | 4 |---+---+
    // | 6 | 9 |---+---+ 2 | 5 |
    // +---+---+ 7 |10 |---+---+
    // |12 |15 |---+---+ 8 |11 |
    // +---+---+13 |16 |---+---+
    //         +---+---+14 |17 |
    //                 +---+---+
    static_assert(flattenIndex<3>({ 1, 1, 1 }, { 3, 2, 3 }) == 10);
    static_assert(expandIndex<3>(10, { 3, 2, 3 }) ==
                  Array<size_t, 3>{ 1, 1, 1 });
    static_assert(flattenIndex<3>({ 0, 1, 2 }, { 3, 2, 3 }) == 5);
    static_assert(expandIndex<3>(5, { 3, 2, 3 }) ==
                  Array<size_t, 3>{ 0, 1, 2 });
}

/// # Class hierarchy of new test cases

namespace {

enum class ID { Animal = 0, Cetacea = 1, Whale = 2, Dolphin = 3, Leopard = 4 };

struct Animal;
struct Cetacea;
struct Whale;
struct Dolphin;
struct Whale;
struct Leopard;

} // namespace

FASTVIS_DEFINE(Animal, ID::Animal, void, Abstract)
FASTVIS_DEFINE(Cetacea, ID::Cetacea, Animal, Abstract)
FASTVIS_DEFINE(Whale, ID::Whale, Cetacea, Concrete)
FASTVIS_DEFINE(Dolphin, ID::Dolphin, Cetacea, Concrete)
FASTVIS_DEFINE(Leopard, ID::Leopard, Animal, Concrete)

namespace {

class Animal: public fastvis::dyn_base_helper<Animal> {
protected:
    constexpr Animal(ID id): dyn_base_helper(id) {}
};

struct Cetacea: Animal {
protected:
    using Animal::Animal;
};

struct Whale: Cetacea {
    constexpr Whale(): Cetacea(ID::Whale) {}
};

} // namespace

// Dolphin and Leopard remain undefined at this point

static void testIsaAndDyncast() {
    static constexpr Whale whale;
    constexpr Animal const* animal = &whale;

    /// Pointers
    static_assert(fastvis::isa<Animal>(animal));
    static_assert(fastvis::isa<Cetacea>(animal));
    static_assert(fastvis::isa<Whale>(animal));
    static_assert(!fastvis::isa<Leopard>(animal));
    static_assert(!fastvis::isa<Dolphin>(animal));

    /// References
    static_assert(fastvis::isa<Animal>(*animal));
    static_assert(fastvis::isa<Cetacea>(*animal));
    static_assert(fastvis::isa<Whale>(*animal));
    static_assert(!fastvis::isa<Leopard>(*animal));
    static_assert(!fastvis::isa<Dolphin>(*animal));

    /// IDs
    static_assert(fastvis::isa<Animal>(ID::Whale));
    static_assert(fastvis::isa<Cetacea>(ID::Whale));
    static_assert(fastvis::isa<Whale>(ID::Whale));
    static_assert(!fastvis::isa<Leopard>(ID::Whale));
    static_assert(!fastvis::isa<Dolphin>(ID::Whale));

    /// Dyncast for good measure
    static_assert(fastvis::dyncast<Animal const*>(animal));
    static_assert(fastvis::dyncast<Cetacea const*>(animal));
    static_assert(fastvis::dyncast<Whale const*>(animal));
}

// Now we define the remaining types

namespace {

struct Dolphin: Cetacea {
    constexpr Dolphin(): Cetacea(ID::Dolphin) {}
};

struct Leopard: Animal {
    constexpr Leopard(): Animal(ID::Leopard) {}
    constexpr ~Leopard() {}
};

} // namespace

static void testVisitation() {
    Dolphin d;
    Leopard l;
    /* Return void */ {
        bool foundDolphin = false;
        // clang-format off
        fastvis::visit((Cetacea&)d, fastvis::overload{
            [&](Dolphin const&) {
                foundDolphin = true;
            },
            [&](Whale const&) {}
        }); // clang-format on
        assert(foundDolphin);
    }
    /* More cases */ {
        bool foundCetaceaAndLeopard = false;
        // clang-format off
        fastvis::visit((Cetacea&)d, (Animal const&)l, fastvis::overload{
            [&](Cetacea const&, Leopard const&) {
                foundCetaceaAndLeopard = true;
            },
            [&](Cetacea const&, Animal const&) {},
            [&](Animal const&, Animal const&) {}
        }); // clang-format on
        assert(foundCetaceaAndLeopard);
    }
    /* Return type deduced as int */ {
        // clang-format off
        auto res = fastvis::visit((Cetacea&)d, (Animal const&)l, fastvis::overload{
            [&](Cetacea const&, Leopard const&) {
                return 1;
            },
            [&](Cetacea const&, Animal const&) { return '\0'; },
            [&](Animal const&, Animal const&) { return (unsigned short)0; }
        }); // clang-format on
        assert(res == 1);
        static_assert(std::is_same_v<decltype(res), int>);
    }
    /* Returns reference */ {
        struct Base {};
        struct Derived: Base {};
        Derived obj;

        // clang-format off
        auto& res = fastvis::visit((Cetacea&)d, (Animal const&)l, fastvis::overload{
            [&](Cetacea const&, Leopard const&) -> Derived& {
                return obj;
            },
            [&](Cetacea const&, Animal const&) -> Base& {
                return obj;
            },
            [&](Animal const&, Animal const&) -> Derived& {
                return obj;
            }
        }); // clang-format on
        assert((std::is_same_v<decltype(res), Base&>));
    }
}

static constexpr bool constexprVisitationReturnVoid() {
    Dolphin d;
    bool foundDolphin = false;
    // clang-format off
    fastvis::visit((Cetacea&)d, fastvis::overload{
        [&](Dolphin const&) { foundDolphin = true; },
        [&](Animal const&) {}
    }); // clang-format on
    return foundDolphin;
}

static_assert(constexprVisitationReturnVoid());

static constexpr int constexprVisitationMultipleArguments() {
    Dolphin d;
    Leopard l;
    return fastvis::visit((Cetacea&)d, (Animal const&)l,
                          fastvis::overload{
                              [&](Cetacea const&, Leopard const&) {
        return 1;
    }, [&](Cetacea const&, Animal const&) { return '\0'; },
                              [&](Animal const&, Animal const&) {
        return (unsigned short)0;
    } }); // clang-format on
}

static_assert(constexprVisitationMultipleArguments() == 1);

/// # Second class hierarchy of old test cases

namespace {

// Base
// ├─ LDerivedA
// │  └─ LDerivedB
// │     └─ LDerivedC
// └─ RDerived

enum class Type {
    Base = 0,
    LDerivedA,
    LDerivedB,
    LDerivedC,
    RDerived,
};

struct Base {
protected:
    explicit Base(Type type): _type(type) {}

public:
    Type type() const { return _type; }

private:
    friend Type get_rtti(Base const& obj) { return obj.type(); }

    Type _type;
};

struct LDerivedA: Base {
protected:
    explicit LDerivedA(Type type): Base(type) {}

public:
    LDerivedA(): Base(Type::LDerivedA) {}
};

struct LDerivedB: LDerivedA {
protected:
    LDerivedB(Type type): LDerivedA(type) {}

    LDerivedB(LDerivedB const&) = delete;

public:
    LDerivedB(): LDerivedA(Type::LDerivedB) {}
};

struct LDerivedC: LDerivedB {
    LDerivedC(): LDerivedB(Type::LDerivedC) {}
};

struct RDerived: Base {
    RDerived(): Base(Type::RDerived) {}
};

} // namespace

FASTVIS_DEFINE(Base, Type::Base, void, Abstract);
FASTVIS_DEFINE(LDerivedA, Type::LDerivedA, Base, Concrete);
FASTVIS_DEFINE(LDerivedB, Type::LDerivedB, LDerivedA, Concrete);
FASTVIS_DEFINE(LDerivedC, Type::LDerivedC, LDerivedB, Concrete);
FASTVIS_DEFINE(RDerived, Type::RDerived, Base, Concrete);

template <typename To, typename From>
static bool canDyncast(From&& from) {
    return requires { dyncast<To>(from); };
}

static void testVisit() {
    LDerivedA a;
    Base& base = a;
    assert(fastvis::visit(base, [](Base& base) { return base.type(); }) ==
           Type::LDerivedA);
}

static void testVisitAbstract() {
    LDerivedA a;
    Base& base = a;
    int i = 0;
    /// Primarily testing successful compilation here.
    /// We wrap \p fastvis::overload in a lambda taking \p auto& to test that
    /// the function doesn't get called with non-sensible arguments even if
    /// semantically possible.
    fastvis::visit(base, [&](auto& b) {
        return fastvis::overload{ [&](LDerivedA&) { i = 1; },
                                  [&](RDerived&) { i = 2; } }(b);
    });
    assert(i == 1);
}

namespace {

template <typename>
struct TagType {};

template <typename A, typename B>
bool operator==(TagType<A>, TagType<B>) {
    return std::is_same_v<A, B>;
}

template <typename U>
inline constexpr TagType<U> T;

} // namespace

static void testVisitReturningReference() {
    LDerivedA a;
    Base& base = a;
    auto f = [&, i = 0](Base&) -> auto& { return i; };
    decltype(auto) result = fastvis::visit(base, f);
    assert(T<decltype(result)> == T<int const&>);
}

static void testVisitReturningReferenceToHierarchy() {
    struct A {};
    struct B: A {};
    LDerivedA a;
    Base& base = a;
    /* Reference */ {
        decltype(auto) result =
            fastvis::visit(base, [b = B{}]<typename T>(T&) -> auto& {
            if constexpr (std::is_same_v<T, LDerivedB>) {
                return static_cast<A const&>(b);
            }
            else {
                return b;
            }
        });
        assert(T<decltype(result)> == T<A const&>);
    }
    /* Pointer */ {
        auto* result = fastvis::visit(base, [b = B{}]<typename T>(T&) -> auto* {
            if constexpr (std::is_same_v<T, LDerivedB>) {
                return static_cast<A const*>(&b);
            }
            else {
                return &b;
            }
        });
        assert(T<decltype(result)> == T<A const*>);
    }
}

static void testVisitSubtree() {
    auto dispatcher = [](LDerivedA& x) {
        return visit(x, fastvis::overload{
                            [](LDerivedA& a) { return 0; },
                            [](LDerivedC& c) { return 1; },
                        });
    };
    LDerivedA a;
    LDerivedB b;
    LDerivedC c;
    assert(dispatcher(a) == 0);
    assert(dispatcher(b) == 0);
    assert(dispatcher(c) == 1);
}

static void testVisitSubtree2() {
    auto dispatcher = [](LDerivedA& x) {
        return fastvis::visit(x, fastvis::overload{
                                     [](LDerivedA& a) { return 0; },
                                     [](LDerivedB& b) { return 1; },
                                 });
    };
    LDerivedA a;
    LDerivedB b;
    LDerivedC c;
    assert(dispatcher(a) == 0);
    assert(dispatcher(b) == 1);
    assert(dispatcher(c) == 1);
}

static void testMDVisit() {
    auto dispatcher = [](Base& b, LDerivedA& x) {
        return fastvis::visit(b, x,
                              fastvis::overload{
                                  [](Base&, LDerivedA& a) { return 0; },
                                  [](Base&, LDerivedB& b) { return 1; },
                                  [](LDerivedB&, LDerivedA& a) { return 2; },
                                  [](LDerivedB&, LDerivedB& b) { return 3; },
                              });
    };
    LDerivedA a;
    LDerivedB b;
    LDerivedC c;
    assert(dispatcher(a, a) == 0);
    assert(dispatcher(a, b) == 1);
    assert(dispatcher(a, c) == 1);
    assert(dispatcher(b, a) == 2);
    assert(dispatcher(b, b) == 3);
    assert(dispatcher(b, c) == 3);
}

static void testIsaAndDyncast2() {
    LDerivedA la;

    assert(fastvis::isa<Base>(la));
    assert(fastvis::isa<LDerivedA>(la));
    assert(!fastvis::isa<LDerivedB>(la));
    assert(!fastvis::isa<RDerived>(la));

    assert(fastvis::dyncast<Base*>(&la) != nullptr);
    assert(fastvis::dyncast<LDerivedA*>(&la) != nullptr);
    assert(fastvis::dyncast<LDerivedB*>(&la) == nullptr);
    assert(!canDyncast<RDerived*>(&la));

    CHECK_NOTHROW(fastvis::dyncast<Base&>(la));
    CHECK_NOTHROW(fastvis::dyncast<LDerivedA&>(la));
    CHECK_THROWS(fastvis::dyncast<LDerivedB&>(la));

    Base const* base = &la;

    assert(fastvis::isa<Base>(*base));
    assert(fastvis::isa<LDerivedA>(*base));
    assert(!fastvis::isa<LDerivedB>(*base));
    assert(!fastvis::isa<RDerived>(*base));

    assert(fastvis::dyncast<Base const*>(base) != nullptr);
    assert(fastvis::dyncast<LDerivedA const*>(base) != nullptr);
    assert(fastvis::dyncast<LDerivedB const*>(base) == nullptr);
    assert(fastvis::dyncast<RDerived const*>(base) == nullptr);

    CHECK_NOTHROW(fastvis::dyncast<Base const&>(*base));
    CHECK_NOTHROW(fastvis::dyncast<LDerivedA const&>(*base));
    CHECK_THROWS(fastvis::dyncast<LDerivedB const&>(*base));
    CHECK_THROWS(fastvis::dyncast<RDerived const&>(*base));

    LDerivedB lb;

    assert(fastvis::isa<Base>(lb));
    assert(fastvis::isa<LDerivedA>(lb));
    assert(fastvis::isa<LDerivedB>(lb));
    assert(!fastvis::isa<RDerived>(lb));

    assert(fastvis::dyncast<Base*>(&lb) != nullptr);
    assert(fastvis::dyncast<LDerivedA*>(&lb) != nullptr);
    assert(fastvis::dyncast<LDerivedB*>(&lb) != nullptr);
    assert(!canDyncast<RDerived*>(&lb));

    CHECK_NOTHROW(fastvis::dyncast<Base&>(lb));
    CHECK_NOTHROW(fastvis::dyncast<LDerivedA&>(lb));
    CHECK_NOTHROW(fastvis::dyncast<LDerivedB&>(lb));

    base = &lb;

    assert(fastvis::isa<Base>(*base));
    assert(fastvis::isa<LDerivedA>(*base));
    assert(fastvis::isa<LDerivedB>(*base));
    assert(!fastvis::isa<RDerived>(*base));

    assert(fastvis::dyncast<Base const*>(base) != nullptr);
    assert(fastvis::dyncast<LDerivedA const*>(base) != nullptr);
    assert(fastvis::dyncast<LDerivedB const*>(base) != nullptr);
    assert(fastvis::dyncast<RDerived const*>(base) == nullptr);

    CHECK_NOTHROW(fastvis::dyncast<Base const&>(*base));
    CHECK_NOTHROW(fastvis::dyncast<LDerivedA const&>(*base));
    CHECK_NOTHROW(fastvis::dyncast<LDerivedB const&>(*base));
    CHECK_THROWS(fastvis::dyncast<RDerived const&>(*base));

    RDerived r;

    assert(fastvis::isa<Base>(r));
    assert(!fastvis::isa<LDerivedA>(r));
    assert(!fastvis::isa<LDerivedB>(r));
    assert(fastvis::isa<RDerived>(r));

    assert(fastvis::dyncast<Base*>(&r) != nullptr);
    assert(!canDyncast<LDerivedA*>(&r));
    assert(!canDyncast<LDerivedB*>(&r));
    assert(fastvis::dyncast<RDerived*>(&r) != nullptr);

    CHECK_NOTHROW(fastvis::dyncast<Base&>(r));
    CHECK_NOTHROW(fastvis::dyncast<RDerived&>(r));

    base = &r;

    assert(fastvis::isa<Base>(*base));
    assert(!fastvis::isa<LDerivedA>(*base));
    assert(!fastvis::isa<LDerivedB>(*base));
    assert(fastvis::isa<RDerived>(*base));

    assert(fastvis::dyncast<Base const*>(base));
    assert(!fastvis::dyncast<LDerivedA const*>(base));
    assert(!fastvis::dyncast<LDerivedB const*>(base));
    assert(fastvis::dyncast<RDerived const*>(base));

    CHECK_NOTHROW(fastvis::dyncast<Base const&>(*base));
    CHECK_THROWS(fastvis::dyncast<LDerivedA const&>(*base));
    CHECK_THROWS(fastvis::dyncast<LDerivedB const&>(*base));
    CHECK_NOTHROW(fastvis::dyncast<RDerived const&>(*base));
}

/// MARK: Hierarchy with two classes

namespace {

enum class SHType { SHBase, SHDerived };

struct SHBase {
protected:
    constexpr SHBase(SHType type): type(type) {}

private:
    constexpr friend SHType get_rtti(SHBase const& base) { return base.type; }

    SHType type;
};

struct SHDerived: SHBase {
    SHDerived(): SHBase{ SHType::SHDerived } {}
};

} // namespace

FASTVIS_DEFINE(SHBase, SHType::SHBase, void, Abstract);
FASTVIS_DEFINE(SHDerived, SHType::SHDerived, SHBase, Concrete);

static void testSmallHierarchy() {
    SHDerived d;
    // clang-format off
    int result = fastvis::visit((SHBase&)d, fastvis::overload{
        [](SHBase const&) { return 0; },
        [](SHDerived const&) { return 1; },
    }); // clang-format on
    assert(result == 1);
}

namespace {

enum class ScopeGuardType { Base, Derived };

struct ScopeGuardBase {
protected:
    constexpr ScopeGuardBase(ScopeGuardType type): type(type) {}

private:
    constexpr friend ScopeGuardType get_rtti(ScopeGuardBase const& base) {
        return base.type;
    }

    ScopeGuardType type;
};

struct ScopeGuardDerived: ScopeGuardBase {
    ScopeGuardDerived(std::function<void()> f):
        ScopeGuardBase{ ScopeGuardType::Derived }, f(f) {}

    ~ScopeGuardDerived() { f(); }

    std::function<void()> f;
};

} // namespace

FASTVIS_DEFINE(ScopeGuardBase, ScopeGuardType::Base, void, Abstract);
FASTVIS_DEFINE(ScopeGuardDerived, ScopeGuardType::Derived, ScopeGuardBase,
               Concrete);

static void testDynDelete() {
    bool destroyed = false;
    std::unique_ptr<ScopeGuardBase, fastvis::dyn_deleter> p(
        new ScopeGuardDerived([&] { destroyed = true; }));
    p.reset();
    assert(destroyed);
}

/// MARK: Error tests

template <typename X, typename Int>
static void errorTestDynConcept() {
    static_assert(!fastvis::impl::Dynamic<int>);
    static_assert(!fastvis::impl::Dynamic<X>);
}

template <typename X, typename Int>
static void errorTestVisit() {
    static_assert(!requires { fastvis::visit(Int{}, [](Int) {}); });
    static_assert(!requires { fastvis::visit(X{}, [](X) {}); });
}

struct X {};
template void errorTestDynConcept<X, int>();
template void errorTestVisit<X, int>();

namespace {

int testFunction(int) {
    return 42;
}

struct TestClass {
    int foo(int) { return 42; }
};

} // namespace

static void testToFunction() {
    auto f = fastvis::overload{ testFunction };
    assert(f(0) == 42);
    auto g = fastvis::overload{ &TestClass::foo };
    TestClass t;
    assert(g(t, 0) == 42);
}

#ifndef FASTVIS_IMPL_TEST_COMPILER_ERRORS
#define FASTVIS_IMPL_TEST_COMPILER_ERRORS 0
#endif

#if FASTVIS_IMPL_TEST_COMPILER_ERRORS

static void visitMissingCase(Cetacea& c) {
    fastvis::visit(c, [](Dolphin&) {});
}

static void convertToInvalidDerived(Cetacea& c) {
    fastvis::dyncast<Leopard const&>(c);
    fastvis::unsafe_cast<Leopard const&>(c);
    fastvis::isa<Leopard>(c);
}

#endif // FASTVIS_IMPL_TEST_COMPILER_ERRORS

/// MARK: Union

static void testDynUnion() {
    fastvis::dyn_union<Animal> animal = Leopard();
    // clang-format off
    int result = animal.visit(fastvis::overload{
        [](Animal const&) { return 0; },
        [](Leopard const&) { return 1; },
    }); // clang-format on
    assert(result == 1);
    auto a2 = animal;
    assert(get_rtti(a2.base()) == ID::Leopard);
    auto a3 = std::move(a2);
    assert(get_rtti(a3.base()) == ID::Leopard);
}

static void testPartialUnion() {
    fastvis::dyn_union<Cetacea> c = Whale();
    // clang-format off
    int result = c.visit(fastvis::overload{
        [](Cetacea const&) { return 0; },
        [](Whale const&) { return 1; },
    }); // clang-format on
    assert(get_rtti(c.get<Cetacea>()) == ID::Whale);
    assert(result == 1);
}

int main() {
    testInternals();
    testIsaAndDyncast();
    testVisitation();
    testVisit();
    testVisitAbstract();
    testVisitReturningReference();
    testVisitReturningReferenceToHierarchy();
    testVisitSubtree();
    testVisitSubtree2();
    testMDVisit();
    testIsaAndDyncast2();
    testSmallHierarchy();
    testDynDelete();
    testToFunction();
    testDynUnion();
    testPartialUnion();
}

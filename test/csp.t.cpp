#define CSP_IMPL_ENABLE_DEBUGGING

#include <functional>
#include <vector>

#include <csp.hpp>

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

// We need this enum in in global scope (not in an anonymous namespace) because
// impl::enumIsValid() behaves differently in this case
enum UnscopedGlobal { UG_A, UG_B, UG_C };

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

static_assert(!csp::impl::enumIsValid<TestEnum, -2>());
static_assert(!csp::impl::enumIsValid<TestEnum, -1>());
static_assert(csp::impl::enumIsValid<TestEnum, TestEnum::A>());
static_assert(csp::impl::enumIsValid<TestEnum, TestEnum::B>());
static_assert(csp::impl::enumIsValid<TestEnum, TestEnum::C>());
static_assert(!csp::impl::enumIsValid<TestEnum, 3>());
static_assert(!csp::impl::enumIsValid<TestEnum, 4>());

static_assert(csp::impl::enumRangeFirst<TestEnum>() == 0);
static_assert(csp::impl::enumRangeLast<TestEnum>() == 3);
static_assert(csp::impl::enumCount<TestEnum>() == 3);

static_assert(csp::impl::enumRangeFirst<Unscoped>() == 0);
static_assert(csp::impl::enumRangeLast<Unscoped>() == 3);
static_assert(csp::impl::enumCount<Unscoped>() == 3);

static_assert(csp::impl::enumRangeFirst<UnscopedGlobal>() == 0);
static_assert(csp::impl::enumRangeLast<UnscopedGlobal>() == 3);
static_assert(csp::impl::enumCount<UnscopedGlobal>() == 3);

// MSVC doesn't compile these. It doesn't matter though, because these are
// internals and never used in the way tested here. These tests only exist for
// rigor.
#ifndef _MSC_VER

static_assert(!csp::impl::enumIsValid<XY::TestEnumNegative, -5>());
static_assert(!csp::impl::enumIsValid<XY::TestEnumNegative, -4>());
static_assert(
    csp::impl::enumIsValid<XY::TestEnumNegative, XY::TestEnumNegative::A>());
static_assert(
    csp::impl::enumIsValid<XY::TestEnumNegative, XY::TestEnumNegative::B>());
static_assert(
    csp::impl::enumIsValid<XY::TestEnumNegative, XY::TestEnumNegative::C>());
static_assert(!csp::impl::enumIsValid<XY::TestEnumNegative, 0>());
static_assert(!csp::impl::enumIsValid<XY::TestEnumNegative, 1>());

static_assert(csp::impl::enumRangeFirst<XY::TestEnumNegative, -128>() == -3);
static_assert(csp::impl::enumRangeLast<XY::TestEnumNegative, -128>() == 0);
static_assert(csp::impl::enumCount<XY::TestEnumNegative>() == 3);

static_assert(csp::impl::enumRangeFirst<StrangeScope<[]<size_t I>() {}>::E>() ==
              0);
static_assert(csp::impl::enumRangeLast<StrangeScope<[]<size_t I>() {}>::E>() ==
              3);
static_assert(csp::impl::enumCount<StrangeScope<[]<size_t I>() {}>::E>() == 3);

#endif // _MSC_VER

/// # Index expansion tests

namespace csp::impl {

template <typename T, size_t N>
constexpr bool operator==(Array<T, N> const& A, Array<T, N> const& B) {
    for (size_t i = 0; i < N; ++i) {
        if (A[i] != B[i]) {
            return false;
        }
    }
    return true;
}

} // namespace csp::impl

static void testInternals() {
    using namespace csp::impl;
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

CSP_DEFINE(Animal, ID::Animal, void, Abstract)
CSP_DEFINE(Cetacea, ID::Cetacea, Animal, Abstract)
CSP_DEFINE(Whale, ID::Whale, Cetacea, Concrete)
CSP_DEFINE(Dolphin, ID::Dolphin, Cetacea, Concrete)
CSP_DEFINE(Leopard, ID::Leopard, Animal, Concrete)

namespace {

class Animal: public csp::base_helper<Animal> {
protected:
    constexpr Animal(ID id): base_helper(id) {}
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
    static_assert(csp::isa<Animal>(animal));
    static_assert(csp::isa<Cetacea>(animal));
    static_assert(csp::isa<Whale>(animal));
    static_assert(!csp::isa<Leopard>(animal));
    static_assert(!csp::isa<Dolphin>(animal));

    /// References
    static_assert(csp::isa<Animal>(*animal));
    static_assert(csp::isa<Cetacea>(*animal));
    static_assert(csp::isa<Whale>(*animal));
    static_assert(!csp::isa<Leopard>(*animal));
    static_assert(!csp::isa<Dolphin>(*animal));

    /// IDs
    static_assert(csp::isa<Animal>(ID::Whale));
    static_assert(csp::isa<Cetacea>(ID::Whale));
    static_assert(csp::isa<Whale>(ID::Whale));
    static_assert(!csp::isa<Leopard>(ID::Whale));
    static_assert(!csp::isa<Dolphin>(ID::Whale));

    /// Dyncast for good measure
    static_assert(csp::dyncast<Animal const*>(animal));
    static_assert(csp::dyncast<Cetacea const*>(animal));
    static_assert(csp::dyncast<Whale const*>(animal));
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
        csp::visit((Cetacea&)d, csp::overload{
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
        csp::visit((Cetacea&)d, (Animal const&)l, csp::overload{
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
        auto res = csp::visit((Cetacea&)d, (Animal const&)l, csp::overload{
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
        auto& res = csp::visit((Cetacea&)d, (Animal const&)l, csp::overload{
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
    csp::visit((Cetacea&)d, csp::overload{
        [&](Dolphin const&) { foundDolphin = true; },
        [&](Animal const&) {}
    }); // clang-format on
    return foundDolphin;
}

static_assert(constexprVisitationReturnVoid());

static constexpr int constexprVisitationMultipleArguments() {
    Dolphin d;
    Leopard l;
    return csp::visit((Cetacea&)d, (Animal const&)l,
                      csp::overload{ [&](Cetacea const&, Leopard const&) {
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

CSP_DEFINE(Base, Type::Base, void, Abstract);
CSP_DEFINE(LDerivedA, Type::LDerivedA, Base, Concrete);
CSP_DEFINE(LDerivedB, Type::LDerivedB, LDerivedA, Concrete);
CSP_DEFINE(LDerivedC, Type::LDerivedC, LDerivedB, Concrete);
CSP_DEFINE(RDerived, Type::RDerived, Base, Concrete);

template <typename To, typename From>
static bool canDyncast(From&& from) {
    return requires { dyncast<To>(from); };
}

static void testVisit() {
    LDerivedA a;
    Base& base = a;
    assert(csp::visit(base, [](Base& base) { return base.type(); }) ==
           Type::LDerivedA);
}

static void testVisitAbstract() {
    LDerivedA a;
    Base& base = a;
    int i = 0;
    /// Primarily testing successful compilation here.
    /// We wrap \p csp::overload in a lambda taking \p auto& to test that
    /// the function doesn't get called with non-sensible arguments even if
    /// semantically possible.
    csp::visit(base, [&](auto& b) {
        return csp::overload{ [&](LDerivedA&) { i = 1; },
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
    decltype(auto) result = csp::visit(base, f);
    assert(T<decltype(result)> == T<int const&>);
}

static void testVisitReturningReferenceToHierarchy() {
    struct A {};
    struct B: A {};
    LDerivedA a;
    Base& base = a;
    /* Reference */ {
        decltype(auto) result =
            csp::visit(base, [b = B{}]<typename T>(T&) -> auto& {
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
        auto* result = csp::visit(base, [b = B{}]<typename T>(T&) -> auto* {
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
        return visit(x, csp::overload{
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
        return csp::visit(x, csp::overload{
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
        return csp::visit(b, x,
                          csp::overload{
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

    assert(csp::isa<Base>(la));
    assert(csp::isa<LDerivedA>(la));
    assert(!csp::isa<LDerivedB>(la));
    assert(!csp::isa<RDerived>(la));

    assert(csp::dyncast<Base*>(&la) != nullptr);
    assert(csp::dyncast<LDerivedA*>(&la) != nullptr);
    assert(csp::dyncast<LDerivedB*>(&la) == nullptr);
    assert(!canDyncast<RDerived*>(&la));

    CHECK_NOTHROW(csp::dyncast<Base&>(la));
    CHECK_NOTHROW(csp::dyncast<LDerivedA&>(la));
    CHECK_THROWS(csp::dyncast<LDerivedB&>(la));

    Base const* base = &la;

    assert(csp::isa<Base>(*base));
    assert(csp::isa<LDerivedA>(*base));
    assert(!csp::isa<LDerivedB>(*base));
    assert(!csp::isa<RDerived>(*base));

    assert(csp::dyncast<Base const*>(base) != nullptr);
    assert(csp::dyncast<LDerivedA const*>(base) != nullptr);
    assert(csp::dyncast<LDerivedB const*>(base) == nullptr);
    assert(csp::dyncast<RDerived const*>(base) == nullptr);

    CHECK_NOTHROW(csp::dyncast<Base const&>(*base));
    CHECK_NOTHROW(csp::dyncast<LDerivedA const&>(*base));
    CHECK_THROWS(csp::dyncast<LDerivedB const&>(*base));
    CHECK_THROWS(csp::dyncast<RDerived const&>(*base));

    LDerivedB lb;

    assert(csp::isa<Base>(lb));
    assert(csp::isa<LDerivedA>(lb));
    assert(csp::isa<LDerivedB>(lb));
    assert(!csp::isa<RDerived>(lb));

    assert(csp::dyncast<Base*>(&lb) != nullptr);
    assert(csp::dyncast<LDerivedA*>(&lb) != nullptr);
    assert(csp::dyncast<LDerivedB*>(&lb) != nullptr);
    assert(!canDyncast<RDerived*>(&lb));

    CHECK_NOTHROW(csp::dyncast<Base&>(lb));
    CHECK_NOTHROW(csp::dyncast<LDerivedA&>(lb));
    CHECK_NOTHROW(csp::dyncast<LDerivedB&>(lb));

    base = &lb;

    assert(csp::isa<Base>(*base));
    assert(csp::isa<LDerivedA>(*base));
    assert(csp::isa<LDerivedB>(*base));
    assert(!csp::isa<RDerived>(*base));

    assert(csp::dyncast<Base const*>(base) != nullptr);
    assert(csp::dyncast<LDerivedA const*>(base) != nullptr);
    assert(csp::dyncast<LDerivedB const*>(base) != nullptr);
    assert(csp::dyncast<RDerived const*>(base) == nullptr);

    CHECK_NOTHROW(csp::dyncast<Base const&>(*base));
    CHECK_NOTHROW(csp::dyncast<LDerivedA const&>(*base));
    CHECK_NOTHROW(csp::dyncast<LDerivedB const&>(*base));
    CHECK_THROWS(csp::dyncast<RDerived const&>(*base));

    RDerived r;

    assert(csp::isa<Base>(r));
    assert(!csp::isa<LDerivedA>(r));
    assert(!csp::isa<LDerivedB>(r));
    assert(csp::isa<RDerived>(r));

    assert(csp::dyncast<Base*>(&r) != nullptr);
    assert(!canDyncast<LDerivedA*>(&r));
    assert(!canDyncast<LDerivedB*>(&r));
    assert(csp::dyncast<RDerived*>(&r) != nullptr);

    CHECK_NOTHROW(csp::dyncast<Base&>(r));
    CHECK_NOTHROW(csp::dyncast<RDerived&>(r));

    base = &r;

    assert(csp::isa<Base>(*base));
    assert(!csp::isa<LDerivedA>(*base));
    assert(!csp::isa<LDerivedB>(*base));
    assert(csp::isa<RDerived>(*base));

    assert(csp::dyncast<Base const*>(base));
    assert(!csp::dyncast<LDerivedA const*>(base));
    assert(!csp::dyncast<LDerivedB const*>(base));
    assert(csp::dyncast<RDerived const*>(base));

    CHECK_NOTHROW(csp::dyncast<Base const&>(*base));
    CHECK_THROWS(csp::dyncast<LDerivedA const&>(*base));
    CHECK_THROWS(csp::dyncast<LDerivedB const&>(*base));
    CHECK_NOTHROW(csp::dyncast<RDerived const&>(*base));
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

CSP_DEFINE(SHBase, SHType::SHBase, void, Abstract);
CSP_DEFINE(SHDerived, SHType::SHDerived, SHBase, Concrete);

static void testSmallHierarchy() {
    SHDerived d;
    // clang-format off
    int result = csp::visit((SHBase&)d, csp::overload{
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

CSP_DEFINE(ScopeGuardBase, ScopeGuardType::Base, void, Abstract);
CSP_DEFINE(ScopeGuardDerived, ScopeGuardType::Derived, ScopeGuardBase,
           Concrete);

static void testDynDelete() {
    bool destroyed = false;
    std::unique_ptr<ScopeGuardBase, csp::dyn_deleter> p(
        new ScopeGuardDerived([&] { destroyed = true; }));
    p.reset();
    assert(destroyed);
}

/// MARK: Error tests

template <typename X, typename Int>
static void errorTestDynConcept() {
    static_assert(!csp::impl::Dynamic<int>);
    static_assert(!csp::impl::Dynamic<X>);
}

template <typename X, typename Int>
static void errorTestVisit() {
    static_assert(!requires { csp::visit(Int{}, [](Int) {}); });
    static_assert(!requires { csp::visit(X{}, [](X) {}); });
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
    auto f = csp::overload{ testFunction };
    assert(f(0) == 42);
    auto g = csp::overload{ &TestClass::foo };
    TestClass t;
    assert(g(t, 0) == 42);
}

#ifndef CSP_IMPL_TEST_COMPILER_ERRORS
#define CSP_IMPL_TEST_COMPILER_ERRORS 0
#endif

#if CSP_IMPL_TEST_COMPILER_ERRORS

static void visitMissingCase(Cetacea& c) {
    csp::visit(c, [](Dolphin&) {});
}

static void convertToInvalidDerived(Cetacea& c) {
    csp::dyncast<Leopard const&>(c);
    csp::unsafe_cast<Leopard const&>(c);
    csp::isa<Leopard>(c);
}

#endif // CSP_IMPL_TEST_COMPILER_ERRORS

/// MARK: Union

static void testDynUnion() {
    csp::dyn_union<Animal> animal = Leopard();
    // clang-format off
    int result = animal.visit(csp::overload{
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
    csp::dyn_union<Cetacea> c = Whale();
    // clang-format off
    int result = c.visit(csp::overload{
        [](Cetacea const&) { return 0; },
        [](Whale const&) { return 1; },
    }); // clang-format on
    assert(get_rtti(c.get<Cetacea>()) == ID::Whale);
    assert(result == 1);
}

static void testRanges() {
#if CSP_IMPL_HAS_RANGES
    Dolphin dolphin;
    Whale whale;
    Leopard leopard;
    std::vector<Animal*> animals = { &dolphin, &whale, &leopard };
    assert(std::ranges::distance(animals | csp::filter<Dolphin>) == 1);
    auto* d = (animals | csp::filter<Dolphin>).front();
    static_assert(std::is_same_v<Dolphin*, decltype(d)>);

    auto refs =
        animals | std::views::transform([](auto* p) -> auto& { return *p; });
    auto& r = (refs | csp::filter<Dolphin>).front();
    static_assert(std::is_same_v<Dolphin&, decltype(r)>);
#endif // CSP_IMPL_HAS_RANGES
}

static void testVisitMostDerivedClass() {
    assert(visit(Leopard{}, [](Leopard const&) { return true; }));
}

namespace ext_del {

enum class ID { A, B };

struct A;
struct B;

csp::unique_ptr<A> makeA(int*);
void do_delete(A&);

} // namespace ext_del

CSP_DEFINE(ext_del::A, ext_del::ID::A, void, Abstract)
CSP_DEFINE(ext_del::B, ext_del::ID::B, ext_del::A, Concrete)

struct ext_del::A: csp::base_helper<A> {
    using base_helper::base_helper;
};

static void testExternalDeletion() {
    int i = 1;
    auto p = ext_del::makeA(&i);
    assert(i == 42);
    p.reset();
    assert(i == 0);
}

struct ext_del::B: A {
    B(int* p): A(ID::B), p(p) {
        if (p)
            *p = 42;
    }
    ~B() {
        if (p)
            *p = 0;
    }
    int* p;
};

csp::unique_ptr<ext_del::A> ext_del::makeA(int* p) {
    return csp::make_unique<B>(p);
}

void ext_del::do_delete(A& a) {
    csp::visit(a, [](auto& a) { delete &a; });
}

static void testUniquePtr() {
    {
        auto p = ext_del::makeA(nullptr);
        assert(csp::isa<ext_del::B>(p));
        auto q = csp::dyncast<ext_del::B>(std::move(p));
    }
    {
        csp::dyncast<ext_del::B>(ext_del::makeA(nullptr));
    }
    {
        csp::unique_ptr<ext_del::A const> p = ext_del::makeA(nullptr);
        csp::cast<ext_del::B>(std::move(p));
        csp::dyncast<ext_del::B>(std::move(p));
    }
}

namespace unscoped {

enum ID { ID_A, ID_B, ID_C };

struct A: csp::base_helper<A, ID> {
    using base_helper::base_helper;
};

struct B: A {
    B(): A(ID_B) {}
};

struct C: A {
    C(): A(ID_C) {}
};

} // namespace unscoped

CSP_DEFINE(unscoped::A, unscoped::ID_A, void, Abstract)
CSP_DEFINE(unscoped::B, unscoped::ID_B, unscoped::A, Concrete)
CSP_DEFINE(unscoped::C, unscoped::ID_C, unscoped::A, Concrete)

void testUnscopedEnum() {
    unscoped::B b;
    unscoped::A& a = b;
    assert(csp::isa<unscoped::B>(a));
    int value = visit(a, csp::overload{
                             [](unscoped::B&) { return 1; },
                             [](unscoped::C&) {
        assert(false);
        return 0;
    },
                         });
    assert(value == 1);
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
    testRanges();
    testVisitMostDerivedClass();
    testExternalDeletion();
    testUniquePtr();
}

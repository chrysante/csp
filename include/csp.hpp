#ifndef CSP_HPP
#define CSP_HPP

#include <bit> // For std::bit_cast
#include <cassert>
#include <cstddef>
#include <memory> // For std::destroy_at and std::unique_ptr
#include <type_traits>
#include <typeinfo> // For std::bad_cast
#include <utility>  // For std::index_sequence

/// User facing macro that declares all mappings defined below
#define CSP_DEFINE(Type, ID, ParentType, Corporeality)                         \
    CSP_IMPL_MAP(Type, ID)                                                     \
    CSP_IMPL_PARENT(Type, ParentType)                                          \
    CSP_IMPL_ABSTRACT(Type, Corporeality)

/// Declares a mapping of \p type to its identifier \p ID
#define CSP_IMPL_MAP(Type, ID)                                                 \
    template <>                                                                \
    [[maybe_unused]] inline constexpr decltype(ID) csp::impl::TypeToID<Type> = \
        ID;                                                                    \
    template <>                                                                \
    struct csp::impl::IDToTypeImpl<ID> {                                       \
        using type = Type;                                                     \
    };

/// Declares a mapping of \p type to its parent type \p Parent
#define CSP_IMPL_PARENT(Type, Parent)                                          \
    template <>                                                                \
    struct csp::impl::TypeToParentImpl<Type> {                                 \
        using type = Parent;                                                   \
    };

/// Declares a mapping of \p type to its corporeality \p CorporealityKind
#define CSP_IMPL_ABSTRACT(Type, CorporealityKind)                              \
    template <>                                                                \
    [[maybe_unused]] inline constexpr csp::impl::Corporeality                  \
        csp::impl::TypeToCorporeality<Type> =                                  \
            csp::impl::Corporeality::CorporealityKind;

namespace csp {

namespace impl {

using std::size_t;

/// # Enum reflection machinery

/// Starting value for scanning enum ranges
#ifndef CSP_IMPL_ENUM_RANGE_MIN
#define CSP_IMPL_ENUM_RANGE_MIN -64
#endif

/// End value for scanning enum ranges
#ifndef CSP_IMPL_ENUM_RANGE_MAX
#define CSP_IMPL_ENUM_RANGE_MAX 128
#endif

#ifdef __GNUC__

template <auto E>
constexpr bool enumIsValidImpl() {
    for (size_t I = sizeof(__PRETTY_FUNCTION__) - 2; I >= 0; --I) {
        if (__PRETTY_FUNCTION__[I] == ')')
            return false;
        if (__PRETTY_FUNCTION__[I] == ':')
            return true;
        if (__PRETTY_FUNCTION__[I] == ' ')
            return true;
    }
    assert(false);
}

template <typename E, auto Value>
constexpr bool enumIsValid() {
    using U = std::underlying_type_t<E>;
    return enumIsValidImpl<std::bit_cast<E>(static_cast<U>(Value))>();
}

#else
#error Unsupported compiler
#endif

template <typename E, long long I, long long End, long long Inc>
constexpr long long enumRangeBound() {
    if constexpr (I == End || enumIsValid<E, I>()) {
        return I;
    }
    else {
        return enumRangeBound<E, I + Inc, End, Inc>();
    }
}

template <typename E, long long Min = CSP_IMPL_ENUM_RANGE_MIN,
          long long Max = CSP_IMPL_ENUM_RANGE_MAX>
constexpr long long enumRangeFirst() {
    return enumRangeBound<E, Min, Max, 1>();
}

template <typename E, long long Min = CSP_IMPL_ENUM_RANGE_MIN,
          long long Max = CSP_IMPL_ENUM_RANGE_MAX>
constexpr long long enumRangeLast() {
    return enumRangeBound<E, Max, Min, -1>() + 1;
}

template <typename E>
constexpr std::size_t enumCount() {
    return static_cast<std::size_t>(enumRangeLast<E>() - enumRangeFirst<E>());
}

/// MARK: - TMP debug utilities

#define CSP_IMPL_CONCAT(a, b)      CSP_IMPL_CONCAT_IMPL(a, b)
#define CSP_IMPL_CONCAT_IMPL(a, b) a##b

#define CSP_IMPL_CTPrintType(...)                                              \
    ::csp::impl::CTTP<__VA_ARGS__> CSP_IMPL_CONCAT(ctPrintVar, __LINE__);

#define CSP_IMPL_CTPrintVal(...)                                               \
    ::csp::impl::CTVP<__VA_ARGS__> CSP_IMPL_CONCAT(ctPrintVar, __LINE__);

template <typename...>
struct CTTP;

template <auto...>
struct CTVP;

/// MARK: - General utilities

[[noreturn]] inline void unreachable() {
#if defined(__GNUC__)
    __builtin_unreachable();
#elif defined(_MSC_VER)
    __assume(false);
#else
    assert(false);
#endif
}

#if defined(__GNUC__)
#define CSP_IMPL_ALWAYS_INLINE __attribute__((always_inline))
#define CSP_IMPL_NODEBUG_IMPL  __attribute__((nodebug))
#elif defined(_MSC_VER)
#define CSP_IMPL_ALWAYS_INLINE __forceinline
#define CSP_IMPL_NODEBUG_IMPL
#else
#define CSP_IMPL_ALWAYS_INLINE
#define CSP_IMPL_NODEBUG_IMPL
#endif

/// We define `CSP_IMPL_NODEBUG` only conditionally so we can disable it in
/// our test and debug code
#if defined(CSP_IMPL_ENABLE_DEBUGGING)
#define CSP_IMPL_NODEBUG
#else
#define CSP_IMPL_NODEBUG CSP_IMPL_NODEBUG_IMPL
#endif

/// Small and simple array implementation to avoid `<array>` dependency
template <typename T, size_t Size>
struct Array {
    constexpr T& operator[](size_t index) { return elems[index]; }
    constexpr T const& operator[](size_t index) const { return elems[index]; }

    T elems[Size];
};

template <
    typename First, typename... Rest,
    typename = std::enable_if_t<(std::is_same_v<First, Rest> && ...), void>>
Array(First, Rest...) -> Array<First, sizeof...(Rest) + 1>;

/// Compile time `Min` and `Max` implementations to avoid `<algorithm>`
/// dependency
template <size_t...>
inline constexpr auto Min = nullptr;

template <size_t N>
inline constexpr size_t Min<N> = N;

template <size_t N, size_t M>
inline constexpr size_t Min<N, M> = N < M ? N : M;

template <size_t N, size_t... M>
inline constexpr size_t Min<N, M...> = Min<N, Min<M...>>;

template <size_t...>
inline constexpr auto Max = nullptr;

template <size_t N>
inline constexpr size_t Max<N> = N;

template <size_t N, size_t M>
inline constexpr size_t Max<N, M> = N > M ? N : M;

template <size_t N, size_t... M>
inline constexpr size_t Max<N, M...> = Max<N, Max<M...>>;

/// https://stackoverflow.com/a/31173086/21285803
template <typename T, typename U>
struct copy_cvref {
private:
    using R = std::remove_reference_t<T>;
    using U1 =
        std::conditional_t<std::is_const<R>::value, std::add_const_t<U>, U>;
    using U2 = std::conditional_t<std::is_volatile<R>::value,
                                  std::add_volatile_t<U1>, U1>;
    using U3 = std::conditional_t<std::is_lvalue_reference<T>::value,
                                  std::add_lvalue_reference_t<U2>, U2>;
    using U4 = std::conditional_t<std::is_rvalue_reference<T>::value,
                                  std::add_rvalue_reference_t<U3>, U3>;

public:
    using type = U4;
};

/// Applies the cv and reference qualifiers of `T` to `U`
template <typename T, typename U>
using copy_cvref_t = typename copy_cvref<T, U>::type;

template <typename T>
struct remove_ref_ptr_cv {
    using type = T;
};
template <typename T>
struct remove_ref_ptr_cv<T const>: remove_ref_ptr_cv<T> {};
template <typename T>
struct remove_ref_ptr_cv<T*>: remove_ref_ptr_cv<T> {};
template <typename T>
struct remove_ref_ptr_cv<T&>: remove_ref_ptr_cv<T> {};
template <typename T>
struct remove_ref_ptr_cv<T&&>: remove_ref_ptr_cv<T> {};

template <typename T>
using remove_ref_ptr_cv_t = typename remove_ref_ptr_cv<T>::type;

/// Enum class to denote invalid type IDs
/// This is used internally to indicate the root of an inheritance hierarchy but
/// should not be visible to users.
enum class InvalidTypeID {};

/// Used to identify the base case in `isaImpl()`
template <auto Fallback>
constexpr decltype(Fallback) ValueOr(auto value) {
    return value;
}

/// Base case
template <auto Fallback>
constexpr decltype(Fallback) ValueOr(InvalidTypeID) {
    return Fallback;
}

template <typename... T>
struct TypeList;

template <template <class> class Pred, typename...>
struct TLFilterImpl;

template <template <class> class Pred, typename Head, typename... Tail>
struct TLFilterImpl<Pred, Head, Tail...>: TLFilterImpl<Pred, Tail...> {};

template <template <class> class Pred, typename Head, typename... Tail>
requires Pred<Head>::value
struct TLFilterImpl<Pred, Head, Tail...> {
    using type =
        typename TLFilterImpl<Pred, Tail...>::type::template Prepend<Head>;
};

template <template <class> class Pred>
struct TLFilterImpl<Pred> {
    using type = TypeList<>;
};

/// Variadic type list useful for template meta programming
template <typename... T>
struct TypeList {
    TypeList() = default;
    constexpr TypeList(T...)
    requires(sizeof...(T) > 0)
    {}

    template <typename U>
    using Append = TypeList<T..., U>;

    template <typename U>
    using Prepend = TypeList<U, T...>;

    template <template <class> class Pred>
    using Filter = typename TLFilterImpl<Pred, T...>::type;
};

template <typename T, typename...>
struct FirstImpl {
    using type = T;
};

/// Evaluates to the first type in the parameter pack \p T...
template <typename... T>
using First = typename FirstImpl<T...>::type;

template <typename>
struct IndexSequenceToArrayImpl;

template <size_t... I>
struct IndexSequenceToArrayImpl<std::index_sequence<I...>> {
    static constexpr Array<size_t, sizeof...(I)> value = { I... };
};

/// Converts the `std::index_sequence` \p IdxSeq to an
/// `Array<size_t, IdxSeq::size()>`
template <typename IdxSeq>
constexpr Array IndexSequenceToArray = IndexSequenceToArrayImpl<IdxSeq>::value;

/// MARK: - General RTTI implementation

/// Different kinds of _corporeality_. Corporeality is meant to denote the
/// 'class' of abstractness and concreteness.
enum class Corporeality { Abstract, Concrete };

/// Common traits of an ID type
template <typename IDType>
struct IDTraits {
    static constexpr size_t count = enumCount<IDType>();

    static constexpr IDType first = IDType(enumRangeFirst<IDType>());
    static constexpr IDType last = IDType(enumRangeLast<IDType>());

    static_assert((std::underlying_type_t<IDType>)first == 0,
                  "Type ID enum must start at zero");
};

/// MARK: - Maps

/// Maps \p T from the type domain to the identifier domain
template <typename T>
constexpr InvalidTypeID TypeToID{};

template <auto>
struct IDToTypeImpl {
    using type = void;
};

/// Maps \p ID from the identifier domain to the type domain
template <auto ID>
using IDToType = typename IDToTypeImpl<ID>::type;

template <typename>
struct TypeToParentImpl;

/// Maps \p T types to its parent type
template <typename T>
using TypeToParent = typename TypeToParentImpl<T>::type;

/// Maps \p ID to the ID of its parent type
template <typename IDType>
constexpr IDType IDToParent(IDType ID) {
    constexpr Array ParentArray = []<size_t... I>(std::index_sequence<I...>) {
        return Array{ ValueOr<IDTraits<IDType>::last>(
            TypeToID<TypeToParent<IDToType<IDType(I)>>>)... };
    }(std::make_index_sequence<IDTraits<IDType>::count>{});
    return ParentArray[(size_t)ID];
}

/// Maps \p T to the ID type of its class hierarchy
template <typename T>
using TypeToIDType = decltype(TypeToID<std::decay_t<T>>);

/// Maps \p T to the number of types in its class hierarchy
template <typename T>
constexpr size_t TypeToBound = IDTraits<TypeToIDType<T>>::count;

template <typename... T>
constexpr Array TypesToBounds = { TypeToBound<T>... };

/// Maps \p T to its corporeality
template <typename T>
constexpr Corporeality TypeToCorporeality = Corporeality{ -1 };

/// Maps \p ID to its corporeality
template <auto ID>
constexpr Corporeality IDToCorporeality = TypeToCorporeality<IDToType<ID>>;

/// `true` if \p ID denotes a concrete type
template <auto ID>
constexpr bool IDIsConcrete = IDToCorporeality<ID> == Corporeality::Concrete;

/// `true` if \p ID denotes an abstract type
template <auto ID>
constexpr bool IDIsAbstract = IDToCorporeality<ID> == Corporeality::Abstract;

/// MARK: Concepts to constrain the public interfaces

template <typename T>
concept DynamicImpl =
    !std::is_same_v<remove_ref_ptr_cv_t<decltype(TypeToID<T>)>,
                    InvalidTypeID> &&
    std::is_class_v<T>;

/// Evaluates to `true` if \p T is registered with the RTTI facilities
/// disregarding any cv-ref qualifiers of \p T
template <typename T>
concept Dynamic = DynamicImpl<remove_ref_ptr_cv_t<T>>;

template <typename T, typename U>
concept SharesTypeHierarchyWithImpl =
    DynamicImpl<T> && DynamicImpl<U> &&
    std::same_as<TypeToIDType<T>, TypeToIDType<U>>;

/// Evaluates to `true` if \p T and \p U are registered with the RTTI facilities
/// disregarding any cv-ref qualifiers and are part of the same class hierarchy.
/// \Note This is implemented by checking if the ID types are the same
template <typename T, typename U>
concept SharesTypeHierarchyWith =
    SharesTypeHierarchyWithImpl<remove_ref_ptr_cv_t<T>, remove_ref_ptr_cv_t<U>>;

template <typename From, typename To>
concept CastableImpl = requires(From* from) { static_cast<To*>(from); };

/// Checks if it is possible to `static_cast` pointer to \p From to pointer to
/// \p To
template <typename From, typename To>
concept Castable =
    CastableImpl<remove_ref_ptr_cv_t<From>, remove_ref_ptr_cv_t<To>>;

template <typename P>
concept DynSmartPtr =
    std::is_class_v<std::remove_cvref_t<P>> && requires(P& p) {
        {
            *p
        } -> Dynamic;
    };

template <typename P>
using PointeeType = std::remove_reference_t<decltype(*std::declval<P>())>;

template <typename P, typename To>
struct RebindSmartPtrImpl;

template <template <typename, typename...> class P, typename T,
          typename... Args, typename To>
struct RebindSmartPtrImpl<P<T, Args...>, To> {
    using type = P<To, Args...>;
};

template <typename P, typename To>
using RebindSmartPtr = typename RebindSmartPtrImpl<P, To>::type;

/// MARK: - isa

/// \Returns `true` if \p TestID is a super class of \p ActualID
/// This function should ideally be `consteval` if we were writing C++23,
/// therefore the `ct` prefix
template <typename IDType>
static constexpr bool ctIsaImpl(IDType TestID, IDType ActualID) {
    /// Poor mans `consteval`
    assert(std::is_constant_evaluated());
    if constexpr (std::is_same_v<IDType, InvalidTypeID>) {
        CSP_IMPL_CTPrintVal(TestID);
        CSP_IMPL_CTPrintVal(ActualID);
    }
    if (ActualID == IDTraits<IDType>::last) {
        return false;
    }
    if (ActualID == TestID) {
        return true;
    }
    return ctIsaImpl(TestID, IDToParent(ActualID));
}

template <typename TestType>
static constexpr auto makeIsaDispatchArray() {
    using IDType = decltype(TypeToID<TestType>);
    return []<size_t... Actual>(std::index_sequence<Actual...>) {
        return Array{ ctIsaImpl(TypeToID<TestType>, IDType(Actual))... };
    }(std::make_index_sequence<IDTraits<IDType>::count>{});
}

template <typename TestType>
static constexpr Array IsaDispatchArray = makeIsaDispatchArray<TestType>();

template <typename Test>
constexpr bool isaIDImpl(auto ID) {
    return impl::IsaDispatchArray<Test>[(size_t)ID];
}

template <typename Test, typename Known>
constexpr bool isaImpl(Known* obj) {
    if (!obj) {
        return false;
    }
    return isaIDImpl<Test>(get_rtti(*obj));
}

template <typename Test, typename Known>
constexpr bool isaImpl(Known& obj) {
    return isaImpl<Test>(&obj);
}

template <typename Test>
requires impl::Dynamic<Test>
struct IsaFn {
    template <typename Known>
    requires std::is_class_v<Test> && SharesTypeHierarchyWith<Known, Test>
    CSP_IMPL_NODEBUG constexpr bool operator()(Known const* obj) const {
        return isaImpl<Test>(obj);
    }

    template <typename Known>
    requires std::is_class_v<Test> && SharesTypeHierarchyWith<Known, Test>
    CSP_IMPL_NODEBUG constexpr bool operator()(Known const& obj) const {
        return isaImpl<Test>(obj);
    }

    template <DynSmartPtr Known>
    requires std::is_class_v<Test> &&
             SharesTypeHierarchyWith<PointeeType<Known>, Test>
    CSP_IMPL_NODEBUG constexpr bool operator()(Known const& obj) const {
        return isaImpl<Test>(std::to_address(obj));
    }

    CSP_IMPL_NODEBUG constexpr bool operator()(TypeToIDType<Test> ID) const
    requires std::is_class_v<Test>
    {
        return isaIDImpl<Test>(ID);
    }
};

/// MARK: - dyncast and cast

template <typename To, typename From>
constexpr To dyncastImpl(From* from) {
    if (isaImpl<std::remove_const_t<std::remove_pointer_t<To>>>(from)) {
        return static_cast<To>(from);
    }
    return nullptr;
}

template <typename To, typename From>
constexpr To dyncastImpl(From& from) {
    using ToNoRef = std::remove_reference_t<To>;
    if (auto* result = dyncastImpl<ToNoRef*>(&from)) {
        return *result;
    }
    throw std::bad_cast();
}

template <typename To>
requires impl::Dynamic<To>
struct DyncastFn {
    template <typename From>
    requires std::is_pointer_v<To> && SharesTypeHierarchyWith<From, To> &&
             Castable<From, To>
    CSP_IMPL_NODEBUG constexpr To operator()(From* from) const {
        return dyncastImpl<To>(from);
    }

    template <typename From>
    requires std::is_reference_v<To> && SharesTypeHierarchyWith<From, To> &&
             Castable<From, To>
    CSP_IMPL_NODEBUG constexpr To operator()(From& from) const {
        return dyncastImpl<To>(from);
    }

    template <DynSmartPtr Known>
    requires std::is_class_v<To> &&
             SharesTypeHierarchyWith<PointeeType<Known>, To> &&
             Castable<PointeeType<Known>*,
                      copy_cvref_t<PointeeType<Known>, To>*> &&
             std::is_rvalue_reference_v<Known&&>
    CSP_IMPL_NODEBUG constexpr RebindSmartPtr<
        Known, copy_cvref_t<PointeeType<Known>, To>>
    operator()(Known&& p) const {
        return RebindSmartPtr<Known, copy_cvref_t<PointeeType<Known>, To>>(
            dyncastImpl<copy_cvref_t<PointeeType<Known>, To>*>(p.release()));
    }
};

template <typename To, typename From>
constexpr To castImpl(From* from) {
    assert(!from || dyncastImpl<To>(from) && "cast failed.");
    return static_cast<To>(from);
}

template <typename To, typename From>
constexpr To castImpl(From& from) {
    using ToNoRef = std::remove_reference_t<To>;
    return *castImpl<ToNoRef*>(&from);
}

template <typename To>
requires impl::Dynamic<To>
struct UnsafeCastFn {
    template <typename From>
    requires std::is_pointer_v<To> && SharesTypeHierarchyWith<From, To> &&
             Castable<From, To>
    CSP_IMPL_NODEBUG constexpr To operator()(From* from) const {
        return castImpl<To>(from);
    }

    template <typename From>
    requires std::is_reference_v<To> && SharesTypeHierarchyWith<From, To> &&
             Castable<From, To>
    CSP_IMPL_NODEBUG constexpr To operator()(From& from) const {
        return castImpl<To>(from);
    }

    template <DynSmartPtr Known>
    requires std::is_class_v<To> &&
             SharesTypeHierarchyWith<PointeeType<Known>, To> &&
             Castable<PointeeType<Known>*,
                      copy_cvref_t<PointeeType<Known>, To>*> &&
             std::is_rvalue_reference_v<Known&&>
    CSP_IMPL_NODEBUG constexpr RebindSmartPtr<
        Known, copy_cvref_t<PointeeType<Known>, To>>
    operator()(Known&& p) const {
        return RebindSmartPtr<Known, copy_cvref_t<PointeeType<Known>, To>>(
            castImpl<copy_cvref_t<PointeeType<Known>, To>*>(p.release()));
    }
};

} // namespace impl

inline namespace ops {

template <typename Test>
inline constexpr impl::IsaFn<Test> isa{};

template <typename To>
inline constexpr impl::DyncastFn<To> dyncast{};

template <typename To>
inline constexpr impl::UnsafeCastFn<To> cast{};

template <typename To>
inline constexpr impl::UnsafeCastFn<To> unsafe_cast{};

} // namespace ops

/// MARK: - visit

namespace impl {

/// Tag type used by the overloads of `visit()` that don't take an explicit
/// return type parameter to instruct `visitImpl()` to deduce the return type
enum class DeduceReturnTypeTag {};

/// Converts the N-dimensional multi-index \p index to a single dimensional flat
/// index according to \p bounds
template <size_t N>
CSP_IMPL_ALWAYS_INLINE constexpr size_t flattenIndex(Array<size_t, N> index,
                                                     Array<size_t, N> bounds) {
    static_assert(N > 0);
    assert(index[0] < bounds[0]);
    size_t acc = index[0];
    for (size_t i = 1; i < N; ++i) {
        assert(index[i] < bounds[i]);
        acc *= bounds[i];
        acc += index[i];
    }
    return acc;
}

/// Converts the a single dimensional flat index \p flatIdex to an N-dimensional
/// multi-index according to \p bounds
template <size_t N>
constexpr Array<size_t, N> expandIndex(size_t flatIndex,
                                       Array<size_t, N> bounds) {
    assert(std::is_constant_evaluated());
    Array<size_t, N> index;
    for (size_t i = 0, j = N - 1; i < N; ++i, --j) {
        size_t m = 1;
        for (size_t k = N - 1; k > i; --k) {
            m *= bounds[k];
        }
        index[i] = flatIndex / m;
        flatIndex -= index[i] * m;
    }
    return index;
}

/// Implementation of `ComputeInvocableIndices`
template <auto ID, typename IDType, size_t Current, size_t Last,
          size_t... InvocableIndices>
struct InvocableIndicesImpl:
    std::conditional_t<
        /// `IsInvocable` =
        IDIsConcrete<(IDType)Current> /// `Current` must be concrete to even be
                                      /// considered invocable
            && ctIsaImpl(ID, (IDType)Current), /// and `Current` must be derived
        /// from `ID`
        InvocableIndicesImpl<ID, IDType, Current + 1, Last, InvocableIndices...,
                             Current>,
        InvocableIndicesImpl<ID, IDType, Current + 1, Last,
                             InvocableIndices...>> {};

/// Base case where `Current == Last`
template <auto ID, typename IDType, size_t Last, size_t... InvocableIndices>
struct InvocableIndicesImpl<ID, IDType, Last, Last, InvocableIndices...> {
    using Indices = std::index_sequence<InvocableIndices...>;
};

/// Computes which dispatch cases have to be generated for a given type `T`,
/// i.e. the list of all type IDs (represented as integers) that could be the
/// runtime type of `T`. This using declaration evaluates to a
/// `std::index_sequence` of the invocable indices
template <typename T>
using ComputeInvocableIndices = typename InvocableIndicesImpl<
    TypeToID<std::decay_t<T>>, TypeToIDType<T>,
    (size_t)IDTraits<TypeToIDType<T>>::first,
    (size_t)IDTraits<TypeToIDType<T>>::last>::Indices;

/// Implementation of `MakeStructuredIndex`
template <size_t FlatInvokeIndex, typename... InvocableIndices>
struct MakeStructuredIndexImpl {
    static constexpr Array ExIndex = expandIndex<sizeof...(
        InvocableIndices)>(FlatInvokeIndex, { InvocableIndices::size()... });

    template <size_t... I>
    static auto makeStructured(std::index_sequence<I...>) {
        return std::index_sequence<
            IndexSequenceToArray<InvocableIndices>[ExIndex[I]]...>{};
    }

    using type = decltype(makeStructured(
        std::make_index_sequence<sizeof...(InvocableIndices)>{}));
};

/// Converts the flat index `FlatInvokeIndex` in the space of _invocable_ types
/// to a structured multi-index in the space of all types. Here "space of
/// invocable types" means
/// `[0,...,NumInvocableTypes<T0>) x ... x  [0,...,NumInvocableTypes<TN>)`
/// where `NumInvocableTypes<T> = ComputeInvocableIndices<T>::size()`
/// This template evaluates to a `std::index_sequence` of the structured indices
template <size_t FlatInvokeIndex, typename... InvocableIndices>
using MakeStructuredIndex =
    typename MakeStructuredIndexImpl<FlatInvokeIndex,
                                     InvocableIndices...>::type;

template <typename R, typename F, typename T, typename StructuredIndex>
struct VisitorCase;

/// Evaluates to the type at index \p I in the class hierarchy of \p U with
/// the same cv-qualifications as \p U
template <typename U, size_t I>
using DerivedAt = copy_cvref_t<U&&, IDToType<(TypeToIDType<U>)I>>;

/// We use a macro here to not repeat ourselves and we don't wrap the
/// expression in a lambda to avoid one unnecessary runtime indirection
/// in debug builds
#define CSP_IMPL_INVOKE_EXPR()                                                 \
    f(static_cast<DerivedAt<T, StructuredIndex>>(t)...)

/// Defines the invocation for one combination of runtime argument types. Users
/// of this class are only interested in the member typedef `type
template <typename R, typename F, typename... T, size_t... StructuredIndex>
struct VisitorCase<R, F, TypeList<T...>,
                   std::index_sequence<StructuredIndex...>> {
    CSP_IMPL_NODEBUG static constexpr R impl(F&& f, T&&... t) {
        using ExprType = decltype(CSP_IMPL_INVOKE_EXPR());
        if constexpr (std::is_same_v<R, void>) {
            CSP_IMPL_INVOKE_EXPR();
        }
        if constexpr (std::is_same_v<ExprType, void>) {
            CSP_IMPL_INVOKE_EXPR();
            /// Unreachable because we invoke UB by leaving a non-void
            /// function without a value.
            unreachable();
        }
        else {
            return CSP_IMPL_INVOKE_EXPR();
        }
    }
};

template <typename F, typename... T, size_t... StructuredIndex>
struct VisitorCase<DeduceReturnTypeTag, F, TypeList<T...>,
                   std::index_sequence<StructuredIndex...>> {
    CSP_IMPL_NODEBUG static constexpr decltype(auto) impl(F&& f, T&&... t) {
        return CSP_IMPL_INVOKE_EXPR();
    }
};

#undef CSP_IMPL_INVOKE_EXPR

template <typename R, typename F, typename T, typename InvocableIndices>
struct MakeVisitorCasesImpl;

template <typename R, typename F, typename... T,
          typename... InvocableIndices // <- Pack of index_sequences with the
          // same size as T that hold the invocable
          // indices for each T
          >
struct MakeVisitorCasesImpl<R, F, TypeList<T...>,
                            TypeList<InvocableIndices...>> {
    /// The total number of invocable cases. This is used to make an index
    /// sequence of this size, create structured indices for every flat index
    /// and create dispatch cases from that
    static constexpr size_t TotalInvocableCases =
        (InvocableIndices::size() * ...);

    template <size_t... FlatInvokeIndex>
    static auto makeCaseTypeList(std::index_sequence<FlatInvokeIndex...>) {
        return TypeList<VisitorCase<
            R, F, TypeList<T...>,
            MakeStructuredIndex<FlatInvokeIndex, InvocableIndices...>>...>{};
    }

    template <size_t... FlatInvokeIndex>
    static auto makeFlatCaseIndexList(std::index_sequence<FlatInvokeIndex...>) {
#if 0
        /// Debug code to print the computed values
        constexpr size_t TestInvokeIndex = 3;
        using StructuredIndex =
        MakeStructuredIndex<TestInvokeIndex, InvocableIndices...>;
        CSP_IMPL_CTPrintType(StructuredIndex);
        constexpr size_t FlatIndex =
        flattenIndex(IndexSequenceToArray<StructuredIndex>,
                     TypesToBounds<T...>);
        CSP_IMPL_CTPrintVal(FlatIndex);
        constexpr Array RestructuredIndex =
        expandIndex<sizeof...(T)>(FlatIndex, { TypeToBound<T>... });
        CSP_IMPL_CTPrintVal(RestructuredIndex);
#endif
        return std::index_sequence<
            flattenIndex(IndexSequenceToArray<MakeStructuredIndex<
                             FlatInvokeIndex, InvocableIndices...>>,
                         TypesToBounds<T...>)...>{};
    }

    using CaseTypeList = decltype(makeCaseTypeList(
        std::make_index_sequence<TotalInvocableCases>{}));

    using FlatCaseIndexList = decltype(makeFlatCaseIndexList(
        std::make_index_sequence<TotalInvocableCases>{}));
};

/// Provides two member typedefs
/// - `CaseTypeList` is a `TypeList` of lambda types for all possible invoke
///   cases that downcast to the runtime argument types. Instances of all types
///   can be converted to function pointers.
/// - `FlatCaseIndexList` is an `std::index_sequence` with the same size as
///   `CaseTypeList` that contains the corresponding flat indices in the space
///   of all possible type combinations.
template <typename R, typename F, typename... T>
struct MakeVisitorCases:
    MakeVisitorCasesImpl<R, F, TypeList<T...>,
                         TypeList<ComputeInvocableIndices<T>...>> {};

template <typename R, typename F, typename T, typename Cases>
struct DeduceReturnTypeImpl {
    using type = R;
};

template <typename F, typename... T, typename... Cases>
struct DeduceReturnTypeImpl<DeduceReturnTypeTag, F, TypeList<T...>,
                            TypeList<Cases...>>:
    std::common_reference<decltype(Cases::impl(std::declval<F&&>(),
                                               std::declval<T&&>()...))...> {};

/// Deduces the common return type of the `visit` expression by applying
/// `std::common_reference` over all possible return types.
template <typename R, typename F, typename T_TypeList, typename Cases_TypeList>
using DeduceReturnType =
    typename DeduceReturnTypeImpl<R, F, T_TypeList, Cases_TypeList>::type;

template <size_t... Indices>
requires(sizeof...(Indices) > 0)
inline constexpr size_t IndexRangeSize = Max<Indices...> - Min<Indices...> + 1;

template <typename ReturnType, typename CaseTypeList,
          typename FlatCaseIndexList>
struct InvokeVisitorCases;

/// This class instantiates an array of function pointers to all invocable
/// visitor cases and provides the `impl()` function to invoke a specific case.
///
/// We only generate function pointers in the smallest possible range of
/// invocable indices to avoid generating large chunks of unused zero bytes in
/// the executable. Therefore we have to subtract an offset (the smallest of the
/// `FlatCaseIndices`) from the runtime computed flat index before dispatching.
template <typename ReturnType, typename... Cases, size_t... FlatCaseIndices>
struct InvokeVisitorCases<ReturnType, TypeList<Cases...>,
                          std::index_sequence<FlatCaseIndices...>> {
    static constexpr size_t FirstFlatInvokeIndex = Min<FlatCaseIndices...>;

    static constexpr size_t NumFlatCaseIndices = sizeof...(FlatCaseIndices);

    /// The size of the function pointer array that is generated
    static constexpr size_t FlatInvokeIndexRangeSize =
        IndexRangeSize<FlatCaseIndices...>;

    /// Function pointer for one invoke case
    template <typename Case, typename F, typename... T>
    CSP_IMPL_NODEBUG static constexpr ReturnType casePtr(F&& f, T&&... t) {
        return Case::impl(static_cast<F&&>(f), static_cast<T&&>(t)...);
    }

    /// Runs a for loop to assign the case index function pointers to avoid a
    /// fold expression. Unlike a for loop fold expressions can run into issues
    /// with the expression nesting depth of the compiler.
    template <typename FuncPtrType>
    static constexpr void assignDispatchArray(
        Array<FuncPtrType, FlatInvokeIndexRangeSize>& DispatchArray,
        Array<size_t, NumFlatCaseIndices> const& indices,
        Array<FuncPtrType, NumFlatCaseIndices> const& casePtrs) {
        for (size_t i = 0; i < NumFlatCaseIndices; ++i) {
            DispatchArray[indices[i] - FirstFlatInvokeIndex] = casePtrs[i];
        }
    }

    /// Computes the array of function pointers
    template <typename FuncPtrType, typename F, typename... T>
    static constexpr auto makeDispatchArray() {
        Array<FuncPtrType, FlatInvokeIndexRangeSize> DispatchArray{};
        assignDispatchArray<FuncPtrType>(DispatchArray, { FlatCaseIndices... },
                                         { casePtr<Cases, F, T...>... });
        return DispatchArray;
    }

    /// The computed array of function pointers
    template <typename F, typename... T>
    static constexpr Array DispatchArray =
        makeDispatchArray<ReturnType (*)(F&&, T&&...), F, T...>();

    /// Subtracts the index offset from \p flatIndex and invokes the function
    /// pointer at that index
    template <typename F, typename... T>
    CSP_IMPL_NODEBUG static constexpr ReturnType impl(size_t flatIndex, F&& f,
                                                      T&&... t) {
        /// We use `.elems` directly here to avoid one function call in debug
        /// builds
        auto* dispatcher =
            DispatchArray<F, T...>.elems[flatIndex - FirstFlatInvokeIndex];
        /// ** Is the type hierarchy defined correctly? **
        /// If not `dispatcher` can be null
        assert(dispatcher);
        return dispatcher(static_cast<F&&>(f), static_cast<T&&>(t)...);
    }
};

template <typename R, typename F, typename... T>
CSP_IMPL_NODEBUG constexpr decltype(auto) visitImpl(F&& f, T&&... t) {
    using CaseTypeList = typename MakeVisitorCases<R, F, T...>::CaseTypeList;
    using FlatCaseIndexList =
        typename MakeVisitorCases<R, F, T...>::FlatCaseIndexList;
    using ReturnType = DeduceReturnType<R, F, TypeList<T...>, CaseTypeList>;

    Array index = { (size_t)get_rtti(t)... };
    size_t flatIndex = flattenIndex(index, TypesToBounds<T...>);
    return InvokeVisitorCases<ReturnType, CaseTypeList,
                              FlatCaseIndexList>::impl(flatIndex,
                                                       static_cast<F&&>(f),
                                                       static_cast<T&&>(t)...);
}

} // namespace impl

inline namespace ops {

template <typename R = impl::DeduceReturnTypeTag, typename F, impl::Dynamic T>
CSP_IMPL_NODEBUG constexpr decltype(auto) visit(T&& t, F&& fn) {
    return impl::visitImpl<R>((F&&)fn, (T&&)t);
}

template <typename R = impl::DeduceReturnTypeTag, typename F, impl::Dynamic T0,
          impl::Dynamic T1>
CSP_IMPL_NODEBUG constexpr decltype(auto) visit(T0&& t0, T1&& t1, F&& fn) {
    return impl::visitImpl<R>((F&&)fn, (T0&&)t0, (T1&&)t1);
}

template <typename R = impl::DeduceReturnTypeTag, typename F, impl::Dynamic T0,
          impl::Dynamic T1, impl::Dynamic T2>
CSP_IMPL_NODEBUG constexpr decltype(auto) visit(T0&& t0, T1&& t1, T2&& t2,
                                                F&& fn) {
    return impl::visitImpl<R>((F&&)fn, (T0&&)t0, (T1&&)t1, (T2&&)t2);
}

template <typename R = impl::DeduceReturnTypeTag, typename F, impl::Dynamic T0,
          impl::Dynamic T1, impl::Dynamic T2, impl::Dynamic T3>
CSP_IMPL_NODEBUG constexpr decltype(auto) visit(T0&& t0, T1&& t1, T2&& t2,
                                                T3&& t3, F&& fn) {
    return impl::visitImpl<R>((F&&)fn, (T0&&)t0, (T1&&)t1, (T2&&)t2, (T3&&)t3);
}

template <typename R = impl::DeduceReturnTypeTag, typename F, impl::Dynamic T0,
          impl::Dynamic T1, impl::Dynamic T2, impl::Dynamic T3,
          impl::Dynamic T4>
CSP_IMPL_NODEBUG constexpr decltype(auto) visit(T0&& t0, T1&& t1, T2&& t2,
                                                T3&& t3, T4&& t4, F&& fn) {
    return impl::visitImpl<R>((F&&)fn, (T0&&)t0, (T1&&)t1, (T2&&)t2, (T3&&)t3,
                              (T4&&)t4);
}

template <typename R = impl::DeduceReturnTypeTag, typename F, impl::Dynamic T0,
          impl::Dynamic T1, impl::Dynamic T2, impl::Dynamic T3,
          impl::Dynamic T4, impl::Dynamic T5>
CSP_IMPL_NODEBUG constexpr decltype(auto)
visit(T0&& t0, T1&& t1, T2&& t2, T3&& t3, T4&& t4, T5&& t5, F&& fn) {
    return impl::visitImpl<R>((F&&)fn, (T0&&)t0, (T1&&)t1, (T2&&)t2, (T3&&)t3,
                              (T4&&)t4, (T5&&)t5);
}

} // namespace ops

/// MARK: - Dynamic deleter impl

namespace impl {

template <typename T>
concept ExternallyDestructible = requires(T& t) { do_destroy(t); };

template <typename T>
concept ExternallyDeletable = requires(T& t) { do_delete(t); };

} // namespace impl

struct dyn_destructor {
    template <typename T>
    requires impl::Dynamic<T> && impl::ExternallyDestructible<T>
    constexpr void operator()(T* object) const {
        assert(object && "object must not be null");
        do_destroy(*object);
    }
    constexpr void operator()(impl::Dynamic auto* object) const {
        assert(object && "object must not be null");
        visit(*object, [](auto& derived) { std::destroy_at(&derived); });
    }
};

/// Calls `std::destroy_at` on the most derived type
inline constexpr dyn_destructor dyn_destroy{};

struct dyn_deleter {
    template <typename T>
    requires impl::Dynamic<T> && impl::ExternallyDeletable<T>
    constexpr void operator()(T* object) const {
        assert(object && "object must not be null");
        do_delete(*object);
    }
    constexpr void operator()(impl::Dynamic auto* object) const {
        assert(object && "object must not be null");
        visit(*object, [](auto& derived) { delete &derived; });
    }
};

/// Calls `delete` on the most derived type
inline constexpr dyn_deleter dyn_delete{};

/// Typedef for `unique_ptr` using `dyn_deleter`
template <typename T>
using unique_ptr = std::unique_ptr<T, dyn_deleter>;

/// `make_unique` implementation creating `csp::unique_ptr`
template <typename T, typename... Args>
requires std::constructible_from<T, Args...>
constexpr unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T((Args&&)args...));
}

/// MARK: Union

namespace impl {

/// Generic union
template <typename Head, typename... Tail>
union UnionImpl {
    constexpr UnionImpl() {}
    constexpr ~UnionImpl() {}

    Head head;
    UnionImpl<Tail...> tail;
};

template <typename Head>
union UnionImpl<Head> {
    constexpr UnionImpl() {}
    constexpr ~UnionImpl() {}

    Head head;
};

/// Recursively traverses the `UnionImpl` \p u and returns the first member
/// whose type satisfies the predicate \p Pred
template <template <class> class Pred, typename U>
constexpr decltype(auto) unionFind(U&& u) {
    using R = copy_cvref_t<U, decltype(u.head)>;
    constexpr bool Found = Pred<R>::value;
    if constexpr (Found) {
        return (R)u.head;
    }
    else {
        return unionFind<Pred>(((U&&)u).tail);
    }
}

/// Defines concrete instances of `unionFind` for different predicates. This is
/// a class because otherwise clang has a hard time accepting the predicate
/// template template parameters
template <typename T>
struct UnionFindHelper {
    template <typename Derived>
    struct DerivedFromFilter:
        std::is_base_of<T, std::remove_cvref_t<Derived>> {};

    template <typename U>
    static constexpr decltype(auto) derivedFrom(U&& u) {
        return unionFind<DerivedFromFilter>((U&&)u);
    }

    template <typename U>
    struct SameAsFilter: std::is_same<T, std::remove_cvref_t<U>> {};

    template <typename U>
    static constexpr decltype(auto) sameAs(U&& u) {
        return unionFind<SameAsFilter>((U&&)u);
    }
};

/// \Returns the member of \p u of type \p T
template <typename T, typename U>
constexpr decltype(auto) unionGet(U&& u) {
    return UnionFindHelper<T>::sameAs((U&&)u);
}

/// `MakeTypeListAll<T>::type` denotes a `TypeList` of all types in the
/// hierarchy of `T`
template <typename T, typename I = std::make_index_sequence<TypeToBound<T>>>
struct MakeTypeListAll;

template <typename T, size_t... I>
struct MakeTypeListAll<T, std::index_sequence<I...>> {
    using type = TypeList<IDToType<(TypeToIDType<T>)I>...>;
};

/// `MakeTypeListDerivedConcrete<T>::type` denotes a `TypeList` of all concrete
/// types derived from `Base`
template <typename Base>
struct MakeTypeListDerivedConcrete {
    template <typename T>
    struct IsConcrete: std::bool_constant<IDIsConcrete<TypeToID<T>>> {};
    template <typename T>
    struct Pred: std::conjunction<IsConcrete<T>, std::is_base_of<Base, T>> {};

    using type = typename MakeTypeListAll<Base>::type::template Filter<Pred>;
};

template <typename Base,
          typename Args = typename MakeTypeListDerivedConcrete<Base>::type>
struct DynUnion;

template <typename Base, typename... Args>
struct DynUnion<Base, TypeList<Args...>> {
    template <typename, typename>
    friend struct DynUnion;

    static constexpr bool NothrowMoveConstructible =
        std::conjunction_v<std::is_nothrow_move_constructible<Args>...>;
    static constexpr bool NothrowMoveAssignable =
        std::conjunction_v<std::is_nothrow_move_assignable<Args>...>;

    template <typename T, typename Impl>
    static copy_cvref_t<Impl, T> getImpl(Impl&& impl) {
        // TODO: static assert that all of Args... that are derived from T have
        // the same offset to T
        using R = copy_cvref_t<Impl&&, T>;
        R result = UnionFindHelper<T>::derivedFrom((Impl&&)impl);
        assert(isa<T>(result));
        return (R)result;
    }

    UnionImpl<Args...> impl;
};

struct UnionNoInit {};

} // namespace impl

/// Typesafe union of types derived from `Base`
///
/// `Base` does not have to be the base of the entire class hierarchy. The union
/// can contain subsets of the hierarchy
template <impl::Dynamic Base>
class dyn_union: impl::DynUnion<Base> {
    using impl::DynUnion<Base>::NothrowMoveConstructible;
    using impl::DynUnion<Base>::NothrowMoveAssignable;

public:
    /// \Returns `csp::visit(FWD(*this), FWD(f))`
    /// @{
    template <typename F>
    constexpr decltype(auto) visit(F&& f) & {
        return csp::visit(base(), (F&&)f);
    }
    template <typename F>
    constexpr decltype(auto) visit(F&& f) const& {
        return csp::visit(base(), (F&&)f);
    }
    template <typename F>
    constexpr decltype(auto) visit(F&& f) && {
        return csp::visit(base(), (F&&)f);
    }
    template <typename F>
    constexpr decltype(auto) visit(F&& f) const&& {
        return csp::visit(base(), (F&&)f);
    }
    /// @}

    /// Constructs the union with the value of derived type \p T
    template <std::derived_from<Base> T>
    constexpr dyn_union(T&& t): dyn_union(impl::UnionNoInit{}) {
        std::construct_at(&impl::unionGet<T>(this->impl), (T&&)t);
    }

    /// Lifetime operations @{
    constexpr dyn_union(dyn_union const& rhs): dyn_union(impl::UnionNoInit{}) {
        rhs.visit([this]<typename T>(T const& rhs) -> void {
            std::construct_at(&impl::unionGet<T>(this->impl), rhs);
        });
    }
    dyn_union& operator=(dyn_union const& rhs) {
        if (this == &rhs) {
            return;
        }
        std::destroy_at(this);
        std::construct_at(this, rhs);
        return *this;
    }
    constexpr dyn_union(dyn_union&& rhs) noexcept(NothrowMoveConstructible):
        dyn_union(impl::UnionNoInit{}) {
        rhs.visit([this]<typename T>(T& rhs) -> void {
            std::construct_at(&impl::unionGet<T>(this->impl), std::move(rhs));
        });
    }
    dyn_union& operator=(dyn_union&& rhs) noexcept(NothrowMoveConstructible &&
                                                   NothrowMoveAssignable) {
        if (this == &rhs) {
            return *this;
        }
        if (get_rtti(base()) == get_rtti(rhs.base())) {
            visit([&]<typename T>(T& This) { This = std::move(rhs.get<T>()); });
            return *this;
        }
        std::destroy_at(this);
        std::construct_at(this, std::move(rhs));
        return *this;
    }
    constexpr ~dyn_union() {
        visit([](auto& This) { std::destroy_at(&This); });
    }
    /// @}

    Base& base() & noexcept { return get<Base>(); }
    Base const& base() const& noexcept { return get<Base>(); }
    Base&& base() && noexcept { return std::move(*this).template get<Base>(); }
    Base const&& base() const&& noexcept {
        return std::move(*this).template get<Base>();
    }

    Base* operator->() noexcept { return &base(); }
    Base const* operator->() const noexcept { return &base(); }

    template <std::derived_from<Base> T>
    T& get() & {
        return impl::DynUnion<Base>::template getImpl<T>(this->impl);
    }
    template <std::derived_from<Base> T>
    T const& get() const& {
        return impl::DynUnion<Base>::template getImpl<T>(this->impl);
    }
    template <std::derived_from<Base> T>
    T&& get() && {
        return impl::DynUnion<Base>::template getImpl<T>(std::move(this->impl));
    }
    template <std::derived_from<Base> T>
    T const&& get() const&& {
        return impl::DynUnion<Base>::template getImpl<T>(std::move(this->impl));
    }

private:
    dyn_union(impl::UnionNoInit) {}
};

/// # Base helper

/// ** Read this if compilation fails here: **
/// If you use this class before the mappings for the class hierarchy are
/// defined, you must provide the `IDType` template argument, because
/// `impl::TypeToIDType<Base>` will be undefined.
template <typename Base, typename IDType = impl::TypeToIDType<Base>>
struct base_helper {
    constexpr base_helper(IDType ID): _id(ID) {}

private:
    friend constexpr IDType get_rtti(base_helper const& This) {
        return This._id;
    }

    IDType _id;
};

/// # Overload

namespace impl {

/// The `toFunction` functions forward class types as is and wrap function
/// pointers in lambdas so it becomes possible to derive from them. Function
/// object case
template <typename F>
requires std::is_class_v<std::remove_reference_t<F>>
constexpr F&& toFunction(F&& f) {
    return (F&&)f;
}

/// Function pointer case
template <typename R, typename... Args>
constexpr auto toFunction(R (*fptr)(Args...)) {
    return [fptr](Args... args) { return fptr((Args&&)args...); };
}

/// Member function pointer case
template <typename R, typename T, typename... Args>
constexpr auto toFunction(R (T::*fptr)(Args...)) {
    return [fptr](T& object, Args... args) {
        return (object.*fptr)((Args&&)args...);
    };
}

/// Return type of `toFunction`
template <typename F>
using ToFunctionT =
    std::remove_reference_t<decltype(toFunction(std::declval<F>()))>;

} // namespace impl

/// Helper class to create overloaded function object on the fly from multiple
/// lambda expressions or other function objects or (member) function pointers
///
/// Can be used like this:
///
///     auto f = overload{
///         [](int) { /* Do something for ints */ },
///         [](float) { /* Do something for floats */ },
///         [](MyClass const&) { /* Do something for MyClass */ },
///     };
///
template <typename... F>
struct overload: impl::ToFunctionT<F>... {
    template <typename... G>
    constexpr overload(G&&... g):
        impl::ToFunctionT<F>(impl::toFunction((G&&)g))... {}
    using impl::ToFunctionT<F>::operator()...;
};

template <typename... F>
overload(F...) -> overload<F...>;

} // namespace csp

/// # range utilities

#if defined __has_include
#if __has_include(<ranges>)
#include <ranges>

#define CSP_IMPL_HAS_RANGES 1

namespace csp {

template <impl::Dynamic T>
inline constexpr auto filter =
    std::views::filter(isa<T>) |
    std::views::transform([]<typename U>(U&& u) -> decltype(auto) {
    if constexpr (std::is_pointer_v<std::remove_cvref_t<U>>) {
        return cast<impl::copy_cvref_t<std::remove_reference_t<U>, T>*>(u);
    }
    else {
        return cast<impl::copy_cvref_t<std::remove_reference_t<U>, T>&>(u);
    }
});

} // namespace csp

#endif // __has_include (<ranges>)
#endif // defined __has_include

/// # Undef

#if !defined(CSP_IMPL_ENABLE_DEBUGGING)

#undef CSP_IMPL_CONCAT
#undef CSP_IMPL_CONCAT_IMPL
#undef CSP_IMPL_CTPrintType
#undef CSP_IMPL_CTPrintVal

#endif // CSP_IMPL_ENABLE_DEBUGGING

#undef CSP_IMPL_ALWAYS_INLINE
#undef CSP_IMPL_NODEBUG
#undef CSP_IMPL_NODEBUG_IMPL

#endif // CSP_HPP

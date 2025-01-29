#include <cassert>
#include <memory>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

#include "animals.hpp"

using namespace csp::ops;

template <typename V>
static std::vector<V> createRandomObjects(size_t N, auto ctor) {
    std::mt19937 gen(std::random_device{}());
    std::vector<V> objects;
    objects.reserve(N);
    for (size_t i = 0; i < N; ++i)
        objects.push_back(ctor(gen));
    return objects;
}

template <typename V>
static V selectingConstructor(std::mt19937& rng) {
    static constexpr size_t Count = csp::hierarchy_size<Animal>;
    std::uniform_int_distribution<int> typeDist(0, Count - 1);
    while (true) {
        auto id = typeDist(rng);
        if (csp::is_abstract_id((ID)id))
            continue;
        static constexpr auto DispatchArray =
            []<size_t... I>(std::index_sequence<I...>) {
            return std::array{ +[](int value) -> V {
                static constexpr ID id = (ID)I;
                if constexpr (!csp::is_abstract_id(id)) {
                    return csp::id_to_type_t<id>(value);
                }
                else {
                    csp::impl::unreachable();
                }
            }... };
        }(std::make_index_sequence<Count>{});
        std::uniform_int_distribution<int> valueDist;
        int value = valueDist(rng);
        return DispatchArray[id](value);
    }
};

using AnimalUnion = csp::dyn_union<Animal>;

template <size_t I, typename... T>
struct AnimalVariantImpl:
    std::conditional_t<
        csp::is_abstract_id((ID)(I - 1)), AnimalVariantImpl<I - 1, T...>,
        AnimalVariantImpl<I - 1, csp::id_to_type_t<(ID)(I - 1)>, T...>> {};

template <typename... T>
struct AnimalVariantImpl<0, T...>: std::type_identity<std::variant<T...>> {};

using AnimalVariant = AnimalVariantImpl<csp::hierarchy_size<Animal>>::type;

static std::vector<AnimalUnion> createRandomObjects(size_t N) {
    return createRandomObjects<AnimalUnion>(N,
                                            selectingConstructor<AnimalUnion>);
}

static std::vector<AnimalVariant> createRandomVariants(size_t N) {
    return createRandomObjects<
        AnimalVariant>(N, selectingConstructor<AnimalVariant>);
}

static constexpr size_t Log2NumObjects = 13;
static constexpr size_t NumObjects = size_t{ 1 } << Log2NumObjects;

static void BM_Baseline(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects].base();
        benchmark::DoNotOptimize(obj);
    }
}
BENCHMARK(BM_Baseline);

static void BM_dynamic_cast(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects].base();
        Cetacea* c = dynamic_cast<Cetacea*>(&obj);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_dynamic_cast);

static void BM_csp_isa(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects].base();
        bool result = isa<Cetacea>(obj);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_csp_isa);

static void BM_csp_dyncast(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects].base();
        Cetacea* c = dyncast<Cetacea*>(&obj);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_csp_dyncast);

static void BM_csp_cast(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects].base();
        Cetacea* c = cast<Cetacea*>(&obj);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_csp_cast);

static constexpr auto AnimalVisitor = [](auto const& animal) {
    return animal.computeValue();
};

static constexpr auto AnimalVisitor2 = [](auto const& a, auto const& b) {
    return a.computeValue() ^ b.computeValue();
};

static void BM_csp_visit(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects].base();
        unsigned value = visit(obj, AnimalVisitor);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(BM_csp_visit);

static void BM_std_variant_visit(benchmark::State& state) {
    auto objects = createRandomVariants(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects];
        unsigned value = std::visit(AnimalVisitor, obj);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(BM_std_variant_visit);

static void BM_VirtualFunction(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj = objects[index++ & Log2NumObjects].base();
        unsigned value = obj.computeValue();
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(BM_VirtualFunction);

static void BM_csp_visit2(benchmark::State& state) {
    auto objects = createRandomObjects(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj1 = objects[index++ & Log2NumObjects].base();
        auto& obj2 = objects[index & Log2NumObjects].base();
        unsigned value = visit(obj1, obj2, AnimalVisitor2);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(BM_csp_visit2);

static void BM_std_variant_visit2(benchmark::State& state) {
    auto objects = createRandomVariants(NumObjects);
    size_t index = 0;
    for (auto _ : state) {
        auto& obj1 = objects[index++ & Log2NumObjects];
        auto& obj2 = objects[index & Log2NumObjects];
        unsigned value = std::visit(AnimalVisitor2, obj1, obj2);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(BM_std_variant_visit2);

BENCHMARK_MAIN();

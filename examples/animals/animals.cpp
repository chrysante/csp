#include "animals/animals.hpp"

#include <array>
#include <iostream>
#include <random>
#include <string>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

using namespace examples;
using namespace fastvis;

std::string getHabitat(Animal const& animal) {
    // clang-format off
    return visit(animal, overload{
        [](Mammal const&) { return "Land"; },
        [](Dolphin const&) { return "Water"; },
        [](Fish const&) { return "Water"; },
        [](Bird const&) { return "Air"; },
    }); // clang-format on
}

// `DynUniquePtr` and `makeDynUnique` are not provided by the library because
// they are quite specific and can easily be defined by the user

template <typename T>
using DynUniquePtr = std::unique_ptr<T, dyn_deleter>;

template <typename T, typename... Args>
requires std::constructible_from<T, Args...>
DynUniquePtr<T> makeDynUnique(Args&&... args) {
    return DynUniquePtr<T>(new T(std::forward<Args>(args)...));
}

static DynUniquePtr<Animal> generateAnimal() {
    static std::mt19937 rng(std::random_device{}());
    static std::array<std::string, 10> const names = {
        "Willie", "Laverne", "Patty", "Alfred",      "Bradley",
        "Rose",   "Tammy",   "Eddie", "Christopher", "Everett",
    };
    std::uniform_int_distribution<size_t> nameDist(0, names.size() - 1);
    switch (std::uniform_int_distribution<>(0, 6)(rng)) {
    case 0: return makeDynUnique<Cat>(names[nameDist(rng)]);
    case 1: return makeDynUnique<Dog>(names[nameDist(rng)]);
    case 2: return makeDynUnique<Dolphin>(names[nameDist(rng)]);
    case 3: return makeDynUnique<Goldfish>(names[nameDist(rng)]);
    case 4: return makeDynUnique<Shark>(names[nameDist(rng)]);
    case 5: return makeDynUnique<Sparrow>(names[nameDist(rng)]);
    case 6: return makeDynUnique<Hawk>(names[nameDist(rng)]);
    default: assert(false);
    }
}

static std::string getTypeName(Animal const& animal) {
    return visit(animal, []<typename T>(T const&) {
#ifndef __GNUC__
        return typeid(T).name();
#else
        int status;
        char* name = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
        auto res = name ? std::string(name) : typeid(T).name();
        std::free(name);
        return res;
#endif
    });
}

static void printHabitats() {
    for (int i = 0; i < 10; ++i) {
        auto animal = generateAnimal();
        std::cout << animal->name() << " is a " << getTypeName(*animal)
                  << " who lives in: " << getHabitat(*animal) << std::endl;
    }
}

static void showcaseOperators() {
    Cat cat("Harald");
    Animal& animal = cat;
    assert(isa<Cat>(animal));
    assert(dyncast<Cat*>(&animal) == &cat);
    assert(dyncast<Dog*>(&animal) == nullptr);
    try {
        dyncast<Dog&>(animal);
        assert(false);
    }
    catch (std::bad_cast const& e) {
        std::cout << e.what() << std::endl;
    }
}

static void showcaseUnion() {
    fastvis::dyn_union<Animal> animal = Cat("Herbert");
    std::cout << animal->name() << std::endl;
}

int main() {
    assert(getHabitat(Cat("Chloe")) == "Land");
    assert(getHabitat(Shark("Sophia")) == "Water");
    assert(getHabitat(Sparrow("Marcus")) == "Air");
    printHabitats();
    showcaseOperators();
}

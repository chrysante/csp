#include "animals.hpp"

#include <iostream>
#include <random>
#include <string>

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
    switch (std::uniform_int_distribution<>(0, 6)(rng)) {
    case 0: return makeDynUnique<Cat>();
    case 1: return makeDynUnique<Dog>();
    case 2: return makeDynUnique<Dolphin>();
    case 3: return makeDynUnique<Goldfish>();
    case 4: return makeDynUnique<Shark>();
    case 5: return makeDynUnique<Sparrow>();
    case 6: return makeDynUnique<Hawk>();
    default: assert(false);
    }
}

static std::string getName(Animal const& animal) {
    return visit(animal, []<typename T>(T const&) { return typeid(T).name(); });
}

static void printHabitats() {
    for (int i = 0; i < 10; ++i) {
        auto animal = generateAnimal();
        std::cout << getName(*animal) << " lives in: " << getHabitat(*animal)
                  << std::endl;
    }
}

static void showcaseOperators() {
    Cat cat;
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

int main() {
    printHabitats();
    showcaseOperators();
}

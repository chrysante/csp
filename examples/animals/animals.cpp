#include "animals.hpp"

#include <array>
#include <iostream>
#include <random>
#include <string_view>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

using namespace examples;
using namespace csp;

constexpr std::string_view getHabitat(Animal const& animal) {
    // clang-format off
    return visit(animal, overload{
        [](Mammal const&) { return "Land"; },
        [](Dolphin const&) { return "Water"; },
        [](Fish const&) { return "Water"; },
        [](Bird const&) { return "Air"; },
    }); // clang-format on
}

static_assert(getHabitat(Cat{}) == "Land");
static_assert(getHabitat(Shark{}) == "Water");
static_assert(getHabitat(Sparrow{}) == "Air");

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
        std::cout << getTypeName(*animal)
                  << " lives in: " << getHabitat(*animal) << std::endl;
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

void showcaseMultipleDispatch(Animal const& animal1, Animal const& animal2) {
    visit(animal1, animal2,
          overload{
              [](Cat const& cat, Dog const& dog) {
        std::cout << "The cat hisses at the dog.\n";
    },
              [](Dog const& dog, Cat const& cat) {
        std::cout << "The dog barks at the cat.\n";
    },
              [](Cat const& cat, Fish const& fish) {
        std::cout << "The cat stares at the fish.\n";
    },
              [](Dog const& dog, Bird const& bird) {
        std::cout << "The dog chases the bird.\n";
    },
              [](Animal const& a1, Animal const& a2) {
        std::cout << "The animals ignore each other.\n";
    },
          });
}

int main() {
    printHabitats();
    showcaseOperators();
    showcaseMultipleDispatch(Cat(), Goldfish());
}

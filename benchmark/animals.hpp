/// This file defines a large hierarchy of animal classes, which are designed to
/// benchmark the performance of our type inspection library under real-world
/// scenarios.
///
/// The purpose of this extensive class hierarchy is to simulate a wide variety
/// of types, providing a comprehensive set of types for benchmarking. By
/// including multiple layers of inheritance, we are able to test how well the
/// library performs when generating and working with a large amount of type
/// inspection data. The file is structured to include both abstract base
/// classes and concrete subclasses, allowing us to test dynamic and static
/// dispatch mechanisms and the overhead associated with each.
///
/// This setup is crucial for assessing the efficiency of type-checking
/// operations, such as `isa`, `dyncast` and `visit`, in a scenario where the
/// number of distinct types is large, mimicking the complexity of real-world
/// applications where large type hierarchies are common. The goal is to
/// evaluate how well the library handles type inspection and dispatching when
/// faced with a high number of types, testing both performance and scalability.

#include <csp.hpp>

// clang-format off
enum class ID {
    Animal,
    
    Mammal,
        Cetacea,
            Whale,
            Dolphin,
        Leopard,
        Lion,
        Tiger,
        Elephant,
        Bat,
        Cheetah,
    
    Insect,
        Ant,
        Spider,
    
    Bird,
        Eagle,
        Parrot,
        Penguin,
        Ostrich,
    
    Reptile,
        Crocodile,
        Snake,
    
    Fish,
        Salmon,
        Shark,
        Trout,
        Pike,
        Tuna,
        Mackerel,
        Swordfish,
        Goldfish,
        Bass,
        Catfish,
        Tilapia,
        Clownfish
};
// clang-format on

// Base animal classes
struct Animal;
struct Mammal;
struct Cetacea;
struct Insect;
struct Bird;
struct Reptile;
struct Fish;

// Concrete animals
struct Whale;
struct Dolphin;
struct Leopard;
struct Lion;
struct Tiger;
struct Elephant;
struct Ant;
struct Spider;
struct Eagle;
struct Parrot;
struct Crocodile;
struct Snake;
struct Salmon;
struct Ostrich;
struct Bat;
struct Cheetah;
struct Shark;
struct Penguin;
struct Trout;
struct Pike;
struct Tuna;
struct Mackerel;
struct Swordfish;
struct Goldfish;
struct Bass;
struct Catfish;
struct Tilapia;
struct Clownfish;

// Register the types with CSP
CSP_DEFINE(Animal, ID::Animal, void, Abstract)
CSP_DEFINE(Mammal, ID::Mammal, Animal, Abstract)
CSP_DEFINE(Cetacea, ID::Cetacea, Mammal, Abstract)

CSP_DEFINE(Whale, ID::Whale, Cetacea, Concrete)
CSP_DEFINE(Dolphin, ID::Dolphin, Cetacea, Concrete)

CSP_DEFINE(Leopard, ID::Leopard, Mammal, Concrete)
CSP_DEFINE(Lion, ID::Lion, Mammal, Concrete)
CSP_DEFINE(Tiger, ID::Tiger, Mammal, Concrete)
CSP_DEFINE(Elephant, ID::Elephant, Mammal, Concrete)
CSP_DEFINE(Bat, ID::Bat, Mammal, Concrete)
CSP_DEFINE(Cheetah, ID::Cheetah, Mammal, Concrete)

CSP_DEFINE(Insect, ID::Insect, Animal, Abstract)
CSP_DEFINE(Ant, ID::Ant, Insect, Concrete)
CSP_DEFINE(Spider, ID::Spider, Insect, Concrete)

CSP_DEFINE(Bird, ID::Bird, Animal, Abstract)
CSP_DEFINE(Eagle, ID::Eagle, Bird, Concrete)
CSP_DEFINE(Parrot, ID::Parrot, Bird, Concrete)
CSP_DEFINE(Penguin, ID::Penguin, Bird, Concrete)
CSP_DEFINE(Ostrich, ID::Ostrich, Bird, Concrete)

CSP_DEFINE(Reptile, ID::Reptile, Animal, Abstract)
CSP_DEFINE(Crocodile, ID::Crocodile, Reptile, Concrete)
CSP_DEFINE(Snake, ID::Snake, Reptile, Concrete)

CSP_DEFINE(Fish, ID::Fish, Animal, Abstract)
CSP_DEFINE(Salmon, ID::Salmon, Fish, Concrete)
CSP_DEFINE(Shark, ID::Shark, Fish, Concrete)
CSP_DEFINE(Trout, ID::Trout, Fish, Concrete)
CSP_DEFINE(Pike, ID::Pike, Fish, Concrete)
CSP_DEFINE(Tuna, ID::Tuna, Fish, Concrete)
CSP_DEFINE(Mackerel, ID::Mackerel, Fish, Concrete)
CSP_DEFINE(Swordfish, ID::Swordfish, Fish, Concrete)
CSP_DEFINE(Goldfish, ID::Goldfish, Fish, Concrete)
CSP_DEFINE(Bass, ID::Bass, Fish, Concrete)
CSP_DEFINE(Catfish, ID::Catfish, Fish, Concrete)
CSP_DEFINE(Tilapia, ID::Tilapia, Fish, Concrete)
CSP_DEFINE(Clownfish, ID::Clownfish, Fish, Concrete)

// Animal class definitions

class Animal: public csp::base_helper<Animal> {
private:
    int val;

protected:
    constexpr Animal(ID id, int value): base_helper(id), val(value) {}

public:
    virtual ~Animal() = default; // Enable RTTI for dynamic_cast

    int value() const { return val; }

    virtual unsigned computeValue() const = 0;
};

struct Mammal: Animal {
protected:
    using Animal::Animal;
};

struct Cetacea: Mammal {
protected:
    using Mammal::Mammal;
};

struct Whale: Cetacea {
    constexpr Whale(int value): Cetacea(ID::Whale, value) {}

    unsigned computeValue() const final { return (unsigned)value() + 42; }
};

struct Dolphin: Cetacea {
    constexpr Dolphin(int value): Cetacea(ID::Dolphin, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 2; }
};

struct Leopard: Mammal {
    constexpr Leopard(int value): Mammal(ID::Leopard, value) {}

    unsigned computeValue() const final { return (unsigned)value() / 2; }
};

struct Lion: Mammal {
    constexpr Lion(int value): Mammal(ID::Lion, value) {}

    unsigned computeValue() const final { return (unsigned)value() + 10; }
};

struct Tiger: Mammal {
    constexpr Tiger(int value): Mammal(ID::Tiger, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 3; }
};

struct Elephant: Mammal {
    constexpr Elephant(int value): Mammal(ID::Elephant, value) {}

    unsigned computeValue() const final { return (unsigned)value() + 100; }
};

struct Bat: Mammal {
    constexpr Bat(int value): Mammal(ID::Bat, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 5; }
};

struct Cheetah: Mammal {
    constexpr Cheetah(int value): Mammal(ID::Cheetah, value) {}

    unsigned computeValue() const final { return (unsigned)value() + 50; }
};

struct Insect: Animal {
protected:
    using Animal::Animal;
};

struct Ant: Insect {
    constexpr Ant(int value): Insect(ID::Ant, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 2; }
};

struct Spider: Insect {
    constexpr Spider(int value): Insect(ID::Spider, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 3; }
};

struct Bird: Animal {
protected:
    using Animal::Animal;
};

struct Eagle: Bird {
    constexpr Eagle(int value): Bird(ID::Eagle, value) {}

    unsigned computeValue() const final { return (unsigned)value() + 20; }
};

struct Parrot: Bird {
    constexpr Parrot(int value): Bird(ID::Parrot, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 4; }
};

struct Penguin: Bird {
    constexpr Penguin(int value): Bird(ID::Penguin, value) {}

    unsigned computeValue() const final { return (unsigned)value() / 2; }
};

struct Ostrich: Bird {
    constexpr Ostrich(int value): Bird(ID::Ostrich, value) {}

    unsigned computeValue() const final { return (unsigned)value() + 30; }
};

struct Reptile: Animal {
protected:
    using Animal::Animal;
};

struct Crocodile: Reptile {
    constexpr Crocodile(int value): Reptile(ID::Crocodile, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 6; }
};

struct Snake: Reptile {
    constexpr Snake(int value): Reptile(ID::Snake, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 7; }
};

struct Fish: Animal {
protected:
    using Animal::Animal;
};

struct Salmon: Fish {
    constexpr Salmon(int value): Fish(ID::Salmon, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 8; }
};

struct Shark: Fish {
    constexpr Shark(int value): Fish(ID::Shark, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 10; }
};

struct Trout: Fish {
    constexpr Trout(int value): Fish(ID::Trout, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 9; }
};

struct Pike: Fish {
    constexpr Pike(int value): Fish(ID::Pike, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 12; }
};

struct Tuna: Fish {
    constexpr Tuna(int value): Fish(ID::Tuna, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 15; }
};

struct Mackerel: Fish {
    constexpr Mackerel(int value): Fish(ID::Mackerel, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 6; }
};

struct Swordfish: Fish {
    constexpr Swordfish(int value): Fish(ID::Swordfish, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 14; }
};

struct Goldfish: Fish {
    constexpr Goldfish(int value): Fish(ID::Goldfish, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 3; }
};

struct Bass: Fish {
    constexpr Bass(int value): Fish(ID::Bass, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 5; }
};

struct Catfish: Fish {
    constexpr Catfish(int value): Fish(ID::Catfish, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 4; }
};

struct Tilapia: Fish {
    constexpr Tilapia(int value): Fish(ID::Tilapia, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 7; }
};

struct Clownfish: Fish {
    constexpr Clownfish(int value): Fish(ID::Clownfish, value) {}

    unsigned computeValue() const final { return (unsigned)value() * 2; }
};

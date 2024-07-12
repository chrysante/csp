#ifndef ANIMALS_HPP
#define ANIMALS_HPP

#include <string>

#include "csp.hpp"

namespace examples {

class Animal;
class Mammal;
class Cat;
class Dog;
class Dolphin;
class Fish;
class Goldfish;
class Shark;
class Bird;
class Sparrow;
class Hawk;

enum class AnimalID {
    Animal,
    Mammal,
    Cat,
    Dog,
    Dolphin,
    Fish,
    Goldfish,
    Shark,
    Bird,
    Sparrow,
    Hawk,
};

} // namespace examples

// clang-format off
CSP_DEFINE(examples::Animal,   examples::AnimalID::Animal,   void,             Abstract)
CSP_DEFINE(examples::Mammal,   examples::AnimalID::Mammal,   examples::Animal, Abstract)
CSP_DEFINE(examples::Cat,      examples::AnimalID::Cat,      examples::Mammal, Concrete)
CSP_DEFINE(examples::Dog,      examples::AnimalID::Dog,      examples::Mammal, Concrete)
CSP_DEFINE(examples::Dolphin,  examples::AnimalID::Dolphin,  examples::Mammal, Concrete)
CSP_DEFINE(examples::Fish,     examples::AnimalID::Fish,     examples::Animal, Abstract)
CSP_DEFINE(examples::Goldfish, examples::AnimalID::Goldfish, examples::Fish,   Concrete)
CSP_DEFINE(examples::Shark,    examples::AnimalID::Shark,    examples::Fish,   Concrete)
CSP_DEFINE(examples::Bird,     examples::AnimalID::Bird,     examples::Animal, Abstract)
CSP_DEFINE(examples::Sparrow,  examples::AnimalID::Sparrow,  examples::Bird,   Concrete)
CSP_DEFINE(examples::Hawk,     examples::AnimalID::Hawk,     examples::Bird,   Concrete)
// clang-format on

namespace examples {

class Animal: public csp::base_helper<Animal> {
protected:
    using base_helper::base_helper;
};

class Mammal: public Animal {
protected:
    using Animal::Animal;
};

class Cat: public Mammal {
public:
    explicit constexpr Cat(): Mammal(AnimalID::Cat) {}
};

class Dog: public Mammal {
public:
    explicit constexpr Dog(): Mammal(AnimalID::Dog) {}
};

class Dolphin: public Mammal {
public:
    explicit constexpr Dolphin(): Mammal(AnimalID::Dolphin) {}
};

class Fish: public Animal {
protected:
    using Animal::Animal;
};

class Goldfish: public Fish {
public:
    explicit constexpr Goldfish(): Fish(AnimalID::Goldfish) {}
};

class Shark: public Fish {
public:
    explicit constexpr Shark(): Fish(AnimalID::Shark) {}
};

class Bird: public Animal {
protected:
    using Animal::Animal;
};

class Sparrow: public Bird {
public:
    explicit constexpr Sparrow(): Bird(AnimalID::Sparrow) {}
};

class Hawk: public Bird {
public:
    explicit constexpr Hawk(): Bird(AnimalID::Hawk) {}
};

} // namespace examples

#endif // ANIMALS_HPP

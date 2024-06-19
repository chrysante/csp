#ifndef ANIMALS_HPP
#define ANIMALS_HPP

#include <string>

#include "fastvis.hpp"

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
FASTVIS_DEFINE(examples::Animal,   examples::AnimalID::Animal,   void,             Abstract)
FASTVIS_DEFINE(examples::Mammal,   examples::AnimalID::Mammal,   examples::Animal, Abstract)
FASTVIS_DEFINE(examples::Cat,      examples::AnimalID::Cat,      examples::Mammal, Concrete)
FASTVIS_DEFINE(examples::Dog,      examples::AnimalID::Dog,      examples::Mammal, Concrete)
FASTVIS_DEFINE(examples::Dolphin,  examples::AnimalID::Dolphin,  examples::Mammal, Concrete)
FASTVIS_DEFINE(examples::Fish,     examples::AnimalID::Fish,     examples::Animal, Abstract)
FASTVIS_DEFINE(examples::Goldfish, examples::AnimalID::Goldfish, examples::Fish,   Concrete)
FASTVIS_DEFINE(examples::Shark,    examples::AnimalID::Shark,    examples::Fish,   Concrete)
FASTVIS_DEFINE(examples::Bird,     examples::AnimalID::Bird,     examples::Animal, Abstract)
FASTVIS_DEFINE(examples::Sparrow,  examples::AnimalID::Sparrow,  examples::Bird,   Concrete)
FASTVIS_DEFINE(examples::Hawk,     examples::AnimalID::Hawk,     examples::Bird,   Concrete)
// clang-format on

namespace examples {

class Animal: public fastvis::base_helper<Animal> {
public:
    std::string const& name() const { return m_name; }

protected:
    Animal(AnimalID ID, std::string name):
        base_helper(ID), m_name(std::move(name)) {}

private:
    std::string m_name;
};

class Mammal: public Animal {
protected:
    using Animal::Animal;
};

class Cat: public Mammal {
public:
    explicit Cat(std::string name): Mammal(AnimalID::Cat, std::move(name)) {}
};

class Dog: public Mammal {
public:
    explicit Dog(std::string name): Mammal(AnimalID::Dog, std::move(name)) {}
};

class Dolphin: public Mammal {
public:
    explicit Dolphin(std::string name):
        Mammal(AnimalID::Dolphin, std::move(name)) {}
};

class Fish: public Animal {
protected:
    using Animal::Animal;
};

class Goldfish: public Fish {
public:
    explicit Goldfish(std::string name):
        Fish(AnimalID::Goldfish, std::move(name)) {}
};

class Shark: public Fish {
public:
    explicit Shark(std::string name): Fish(AnimalID::Shark, std::move(name)) {}
};

class Bird: public Animal {
protected:
    using Animal::Animal;
};

class Sparrow: public Bird {
public:
    explicit Sparrow(std::string name):
        Bird(AnimalID::Sparrow, std::move(name)) {}
};

class Hawk: public Bird {
public:
    explicit Hawk(std::string name): Bird(AnimalID::Hawk, std::move(name)) {}
};

} // namespace examples

#endif // ANIMALS_HPP

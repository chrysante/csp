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

FASTVIS_DEFINE(examples::Animal, examples::AnimalID::Animal, void, Abstract)
FASTVIS_DEFINE(examples::Mammal, examples::AnimalID::Mammal, examples::Animal,
               Abstract)
FASTVIS_DEFINE(examples::Cat, examples::AnimalID::Cat, examples::Mammal,
               Concrete)
FASTVIS_DEFINE(examples::Dog, examples::AnimalID::Dog, examples::Mammal,
               Concrete)
FASTVIS_DEFINE(examples::Dolphin, examples::AnimalID::Dolphin, examples::Mammal,
               Concrete)
FASTVIS_DEFINE(examples::Fish, examples::AnimalID::Fish, examples::Animal,
               Abstract)
FASTVIS_DEFINE(examples::Goldfish, examples::AnimalID::Goldfish, examples::Fish,
               Concrete)
FASTVIS_DEFINE(examples::Shark, examples::AnimalID::Shark, examples::Fish,
               Concrete)
FASTVIS_DEFINE(examples::Bird, examples::AnimalID::Bird, examples::Animal,
               Abstract)
FASTVIS_DEFINE(examples::Sparrow, examples::AnimalID::Sparrow, examples::Bird,
               Concrete)
FASTVIS_DEFINE(examples::Hawk, examples::AnimalID::Hawk, examples::Bird,
               Concrete)

namespace examples {

class Animal {
protected:
    Animal(AnimalID ID): ID(ID) {}

private:
    friend AnimalID get_rtti(Animal const& animal) { return animal.ID; }

    AnimalID ID;
};

class Mammal: public Animal {
protected:
    using Animal::Animal;
};

class Cat: public Mammal {
public:
    Cat(): Mammal(AnimalID::Cat) {}
};

class Dog: public Mammal {
public:
    Dog(): Mammal(AnimalID::Dog) {}
};

class Dolphin: public Mammal {
public:
    Dolphin(): Mammal(AnimalID::Dolphin) {}
};

class Fish: public Animal {
protected:
    Fish(AnimalID ID, float size): Animal(ID), m_size(size) {}

private:
    float m_size;
};

class Goldfish: public Fish {
public:
    Goldfish(float size = 1): Fish(AnimalID::Goldfish, size) {}
};

class Shark: public Fish {
public:
    Shark(float size = 100): Fish(AnimalID::Shark, size) {}
};

class Bird: public Animal {
protected:
    using Animal::Animal;
};

class Sparrow: public Bird {
public:
    Sparrow(): Bird(AnimalID::Sparrow) {}
};

class Hawk: public Bird {
public:
    Hawk(): Bird(AnimalID::Hawk) {}
};

} // namespace examples

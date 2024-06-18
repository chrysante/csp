#  RTTI library for fast visitation, pattern matching and multiple dispatch

This is a support library I've written for my compiler project. 
It is inspired by LLVM's RTTI, but goes a step further by allowing `std::variant`-like `visit` expressions. 

The setup of the RTTI-data is a bit involved, so before scaring you off, 
let me show you the benefits.

### Pattern matching example

Say you have a class hierarchy of animals and you want to map different kinds of animals to their habitats.
This library allows you to use a pattern matching style `visit` expression for this task:

    #include "animals.h"

    std::string getHabitat(Animal const& animal) {
        return fastvis::visit(animal, fastvis::overload{
            [](Mammal const&) { return "Land"; },
            [](Dolphin const&) { return "Water"; },
            [](Fish const&) { return "Water"; },
            [](Bird const&) { return "Air"; },
        });
    }

    assert(getHabitat(Cat{}) == "Land");
    assert(getHabitat(Shark{}) == "Water");
    assert(getHabitat(Sparrow{}) == "Air");

This is the `Animal` class hierarchy defined in `"animals.hpp"`

    Animal 
    ├─ Mammal
    │  ├─ Cat 
    │  ├─ Dog
    │  └─ Dolphin
    ├─ Fish
    │  ├─ Goldfish
    │  └─ Shark
    └─ Bird
       ├─ Sparrow
       └─ Hawk
    
Note that we don't have to list every single class in the `visit` expression. If
several child classes have the same behaviour, 
it is sufficient to define the behaviour for a base class. This means we can 
only mention the `Fish` class to define the behaviour for `Goldfish` and 
`Shark`, both of whom live in water. 
The same goes for `Sparrow` and `Hawk`, whose behaviour is defined only for 
the `Bird` parent class.
Most mammals live on land, so we define `Mammal` to return `"Land"`, but add a special case 
for `Dolphin`.

### Another classical OOP example: shapes!

We have three classes, `Circle`, `Rect`, `Triangle`, and a common base class `Shape`

We define collision behaviour for all combinations of shapes:

    #include "Shapes.hpp"

    void doCollide(Circle const&, Circle const&);
    void doCollide(Circle const&, Rect const&);
    void doCollide(Circle const&, Triangle const&);
    void doCollide(Rect const&, Rect const&);
    void doCollide(Rect const&, Triangle const&);
    void doCollide(Triangle const&, Triangle const&);
    void doCollide(std::derived_from<Shape> auto const& a, 
                   std::derived_from<Shape> auto const& b) {
        return doCollide(b, a);
    }

Now, with two references to `Shapes`, we can again use `visit` to dispatch to the correct implementation:
    
    void collide(Shape const& a, Shape const& b) {
        fastvis::visit(a, b, [](auto const& a, auto const& b) {
            collideImpl(a, b);
        });
    }

This is useful if we have a list of `Shape` pointers for which we want to handle collision: 

    void handleCollisions(std::span<Shape* const> shapes) {
        for (size_t i = 0; i < shapes.size(); ++i) {
            for (size_t j = i + 1; j < shapes.size(); ++j) {
                collide(*shapes[i], *shapes[j]);
            }
        }
    }
    
### Operators

The library provides three type inspection "operators": `isa`, `dyncast` and `cast`

- `isa` tests if a pointer or reference to a base class is an instance of a specific derived class:
    
        Cat cat;
        Animal& animal = cat;
        assert(fastvis::isa<Cat>(animal));
    
- `dyncast` works just like `dynamic_cast`. 
Casting pointers returns `nullptr` if the cast failed, or a pointer to the derived type. Casting references throws `std::bad_cast` on failure.
    
        Cat cat;
        Animal* animal = &cat;
        assert(fastvis::dyncast<Cat*>(animal) == &cat);
        assert(fastvis::dyncast<Dog*>(animal) == nullptr);
         
        try {
            fastvis::dyncast<Dog&>(*animal);
        }
        catch (std::bad_cast const&) {}
    
- `cast` works like `dyncast`, except that a failed cast results in undefined behaviour. It should rarely be used and can be thought of as a replacement for `static_cast` to a derived type.
If `NDEBUG` is defined (release builds), the library assumes that all `cast`'s succeed. Otherwise (in debug builds) the library asserts that `cast` succeeds. 

## Setup (the ugly part)

The code above is nice, but it doesn't come for free. For `isa`, `dyncast`, `cast` and `visit` to work with a class hierarchy, a few mappings need to be defined:

    // Forward declarations of all classes in the hierarchy
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

    // Enum listing all classes in the hierarchy
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

    // Bindings to the library, in the global namespace:
    //           - Type    - Enum ID           - Parent class   - Abstract or Concrete
    FASTVIS_DEFINE(Animal,   AnimalID::Animal,   void,            Abstract)
    FASTVIS_DEFINE(Mammal,   AnimalID::Mammal,   Animal,          Abstract)
    FASTVIS_DEFINE(Cat,      AnimalID::Cat,      Mammal,          Concrete)
    FASTVIS_DEFINE(Dog,      AnimalID::Dog,      Mammal,          Concrete)
    FASTVIS_DEFINE(Dolphin,  AnimalID::Dolphin,  Mammal,          Concrete)
    FASTVIS_DEFINE(Fish,     AnimalID::Fish,     Animal,          Abstract)
    FASTVIS_DEFINE(Goldfish, AnimalID::Goldfish, Fish,            Concrete)
    FASTVIS_DEFINE(Shark,    AnimalID::Shark,    Fish,            Concrete)
    FASTVIS_DEFINE(Bird,     AnimalID::Bird,     Animal,          Abstract)
    FASTVIS_DEFINE(Sparrow,  AnimalID::Sparrow,  Bird,            Concrete)
    FASTVIS_DEFINE(Hawk,     AnimalID::Hawk,     Bird,            Concrete)

The `FASTVIS_DEFINE` macro defines mappings for each class in the hierarchy to its base class, 
to its runtime type ID (and back), and it defines each class as abstract or concrete.

This last part may seem strange, but it is required for `visit` expressions:
If all concrete derived classes of a particular class are matched, there is no need to match the base class. 
But this only works if the base class cannot be instantiated, i.e., if it's abstract. Because the library 
doesn't require the classes to have virtual functions, this cannot be implemented via `std::is_abstract`.  

One more thing the library needs, is the actual runtime type information. This is achieved with the enum defined above.

The base class of the hierarchy inherits from `fastvis::base_helper` to handle the RTTI:

    class Animal: public fastvis::base_helper<Animal> {
    protected:
        Animal(AnimalID ID): base_helper(ID) {}   
    };
    
Inheriting from `fastvis::base_helper` is not required, you can also manage the RTTI yourself.
To expose the type identifiers to the library, define a function `get_rtti` accessible via ADL:
    
    class Animal {
    protected:
        Animal(AnimalID ID): ID(ID) {}
        
    private:
        friend AnimalID get_rtti(Animal const& animal) { return animal.ID; }
    
        AnimalID ID;
    };
    
Then you can define all other classes as you normally would 
    
    class Mammal: public Animal {
    protected:
        using Animal::Animal;   
    };
    
    class Fish: public Animal {
    protected:
        Fish(AnimalID ID, float size): Animal(ID), m_size(size) {}
    
    private:
        float m_size;   
    };
    
Note that all concrete classes must pass their type ID to the constructor of 
their parent class
    
    class Cat: public Mammal {
    public:
        Cat(): Mammal(AnimalID::Cat) {}
    };
    
    // ...
    
### Incomplete types

`isa` works with incomplete types. This is why the parent class must be specified in the `FASTVIS_DEFINE` macro.
The alternative was that all classes in the hierarchy must be complete for `isa` and `dyncast` to work, which 
can be bad for compile times, so I decided to put the burden on the user to specify the parent relationship here.
The library asserts that the specified parent class is actually a parent class whenever possible.  

## Comparison to the OOP visitor pattern 

All types in a class hierarchy must be known at compile time. 
This is drawback compared to classical polymorphism, where additional derived types can 
even be defined by third party plugin libraries. 

However, in the OOP visitor pattern, all types must be known at compile time as 
well. Also, a lot of boiler plate similar to the setup code required for this
library is required by the visitor pattern. Unlike the visitor pattern, the 
dynamic dispatch performed by this library is very fast and allows ad-hoc 
definition of visitors. No need to derive from a `Visitor` class and no double 
indirect call is required.

On top of that, it makes multiple dispatch trivial. Simply pass multiple 
arguments to `visit` and let the vistor take multiple parameters.  

## Comparison to `std::visit(std::variant)`

You may have noticed that everything described above is very 
similar to `std::visit(std::variant)`. The implementation is  similar as well, and if `std::variant` solves your problems, it's probably best to 
stick to that.  

However, `std::variant` has a major drawback: It is 
completely unstructured. A `std::variant<Dog, Cat, Dolphin, ...>` is a flat list
of all contained types. All you can do to meaningfully access the contained 
value, is `std::visit` or `std::get` and handle each type separately. 
With this library you always have an `Base&` or `Base*` and you can access 
the interface of the base class. Also you can define behaviours for base classes
 of subsets, like `Mammal` or `Fish` in the example above. 

## Performance

Very similar to `std::variant`. 

`isa` is implemented with a lookup table built at compile time. The information 
needed to build the lookup table is defined by the user with the `FASTVIS_DEFINE` 
macro.

`cast` and `dyncast` internally use `isa` followed by a `static_cast`. 

Here are sketch implementations of the various functions: 

    namespace fastvis {

    template <typename Derived, typename Base>
    bool isa(Base const& object) { 
        static constexpr bool LookupTable[/* num-classes */] = /* ... */;
        return LookupTable[object./*runtime-id*/]; 
    }
    
    template <typename Derived, typename Base>
    bool isa(Base const* object) {
        return object && isa<Derived>(*object);
    }

    template <typename Derived, typename Base>
    Derived dyncast(Base* object) {
        if (isa</*remove ref/pointer and cv* /Derived>(object)) {
            return static_cast<Derived>(object);
        }
        return nullptr;
    }
    
    template <typename Derived, typename Base>
    Derived dyncast(Base& object) {
        if (isa</*remove ref/pointer and cv* /Derived>(object)) {
            return static_cast<Derived>(object);
        }
        throw std::bad_cast();
    }

    template <typename T, typename Visitor>
    decltype(auto) visit(T& object, Visitor&& vis) {
        using FuncPtr = /*common-return-type*/(*)(T&, Visitor&&);
        static constexpr FuncPtr Table[/* num-classes */] = /* ... */;
        return Table[object./*runtime-id*/](object, std::forward<Visitor>(vis));
    }
    
    }

## Utilities

Accessing polymorhic objects is one thing, storing them is another. 
For storing polymorphic types in containers of `std::unique_ptr`, this library provides the `dyn_deleter` 
class to safely `delete` allocated objects through pointers-to-base without virtual destructors. 

    {
        std::vector<std::unique_ptr<Animal, fastvis::dyn_deleter>> v;
        v.push_back(std::unique_ptr<Animal, fastvis::dyn_deleter>(new Dolphin()));
    } // dyn_deleter visits the most derived type and calls `delete` on it
    
Note that if the base class has a virtual destructor, this is not required, and you can use a normal `std::unique_ptr` 
to store objects. `dyn_deleter` allows you to elide the vtable pointer from your objects, if it would only be used for the destructor. 

If you don't want to dynamically allocate your objects, you can use the `dyn_union` template to create a union of all types in a class hierarchy:

    fastvis::dyn_union<Animal> animal = Cat{};
    static_assert(sizeof(fastvis::dyn_union<Animal>) == std::max({ sizeof(Cat), sizeof(Dog), ... }));

You can always use an unqualified call to `get_rtti` to get the runtime type ID of an object:
    
    Cat cat;
    Animal& animal = cat;
    assert(get_rtti(animal) == AnimalID::Cat);

In my projects where I use this library, I bring the operators into my project scope so I don't have to qualify calls to the operators:
 
    #include <fastvis.hpp>
 
    namespace MyProject {
         using fastvis::isa;
         using fastvis::dyncast;
         using fastvis::cast;
         using fastvis::visit;
    }

### Ranges

A few range utilities are provided: 

    std::vector<Animal*> animals = /* ... */;
    
    // Filters the birds and casts the range elements to `Bird*`
    for (Bird* bird: animals | fastvis::filter<Bird>) {
    
    }

The `isa`, `dyncast` and `cast` operators are function objects, so they works nicely with the `<ranges>` and `<algorithm>` libraries:
 
    std::vector<Animal*> animals = /* ... */;
    
    // Find a cat in `animals`
    auto itr = std::find_if(animals.begin(), animals.end(), fastvis::isa<Cat>);
    
    if (std::ranges::all_of(animals, fastvis::isa<Mammal>)) {
        // All animals are mammals
    }
    
    // View of `Bird` pointers (possibly null)
    auto birds = animals | std::views::transform(fastvis::dyncast<Bird*>);

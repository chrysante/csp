#ifndef UTILS_HPP
#define UTILS_HPP

#include <concepts>
#include <exception>

namespace examples {

enum class Format { Red, Green, Blue, Grey, Bold, Reset };

void format(Format f);

void format(std::same_as<Format> auto... f) {
    (format(f), ...);
}

struct QuitException: std::exception {
    char const* what() const noexcept { return "quit"; }
};

struct InputException: std::exception {
    char const* what() const noexcept { return "Input error"; }
};

} // namespace examples

#endif // UTILS_HPP

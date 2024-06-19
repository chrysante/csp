#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include "ast.hpp"

namespace examples {

DynUniquePtr<Program> parse(std::string source);

} // namespace examples

#endif // PARSER_HPP

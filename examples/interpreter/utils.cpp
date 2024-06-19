#include "utils.hpp"

#include <cstdio>

using namespace examples;

void examples::format(Format f) {
    switch (f) {
    case Format::Red:
        printf("\033[91m");
        break;
    case Format::Green:
        printf("\033[32m");
        break;
    case Format::Blue:
        printf("\033[94m");
        break;
    case Format::Grey:
        printf("\033[90m");
        break;
    case Format::Bold:
        printf("\033[1m");
        break;
    case Format::Reset:
        printf("\033[00m");
        break;
    }
}

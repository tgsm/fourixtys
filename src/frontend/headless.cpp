#include <fmt/core.h>
#include "frontend/headless.h"

int main_headless(std::span<std::string_view> args) {
    fmt::print("Hello, friend\n");
    return 0;
}

#include <fmt/core.h>
#include "frontend/headless.h"
#include "n64.h"

int main_headless(std::span<std::string_view> args) {
    PIF pif(args[0]);
    GamePak gamepak(args[1]);

    if (!gamepak.swap_bytes_for_endianness()) {
        return 1;
    }

    N64 n64(pif, gamepak);

    fmt::print("Hello, friend\n");
    return 0;
}

#include <fmt/core.h>
#include <vector>
#include "frontend/frontend.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fmt::print("usage: {} <pif> <gamepak>\n", argv[0]);
        return 1;
    }

    std::vector<std::string_view> args {};
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    return main_headless(args);
}

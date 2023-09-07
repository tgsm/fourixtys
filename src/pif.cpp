#include <fstream>
#include <fmt/std.h>
#include "pif.h"

PIF::PIF(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        fmt::print("fatal: Provided PIF is not a regular file: '{}'\n", path);
        std::abort();
    }

    if (std::filesystem::file_size(path) != PifSize) {
        fmt::print("fatal: Provided PIF is not {} bytes: '{}'\n", PifSize, path);
        std::abort();
    }

    std::ifstream stream(path, std::ios::binary);
    if (!stream.good()) {
        fmt::print("fatal: Could not open provided PIF: '{}'\n", path);
        std::abort();
    }

    stream.read(reinterpret_cast<char*>(m_pif.data()), PifSize);
}

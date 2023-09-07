#include <bit>
#include <fstream>
#include <fmt/core.h>
#include <fmt/std.h>
#include "gamepak.h"

static constexpr u32 Z64_IDENTIFIER = 0x80371240;
static constexpr u32 N64_IDENTIFIER = 0x37804012;

GamePak::GamePak(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        fmt::print("fatal: Provided GamePak is not a regular file: '{}'\n", path);
        std::abort();
    }

    const std::size_t file_size = std::filesystem::file_size(path);
    if (file_size < 0x40) {
        fmt::print("fatal: Provided GamePak is not big enough: {}\n", path);
        std::abort();
    }

    if (file_size > 0xFBFFFFF) {
        fmt::print("fatal: Provided GamePak is too big: {}\n", path);
        std::abort();
    }

    std::ifstream stream(path, std::ios::binary);
    if (!stream.good()) {
        fmt::print("fatal: Could not open provided GamePak: {}\n", path);
    }

    m_rom.resize(file_size);
    stream.read(reinterpret_cast<char*>(m_rom.data()), m_rom.size());
}

bool GamePak::swap_bytes_for_endianness() {
    const u32 identifier = read<u32>(0);

    if (identifier == Z64_IDENTIFIER) {
        fmt::print("Z64 ROM format, no byteswapping required\n");
        return true;
    }

    if (identifier == N64_IDENTIFIER) {
        fmt::print("N64 ROM format, byteswapping...\n");

        for (std::size_t i = 0; i < m_rom.size(); i += sizeof(u16)) {
            u16* word = reinterpret_cast<u16*>(&m_rom.at(i));
            *word = __builtin_bswap16(*word);
        }

        fmt::print("done byteswapping\n");
        return true;
    }

    fmt::print("Unrecognized ROM format identifier {:08X}\n", identifier);
    return false;
}

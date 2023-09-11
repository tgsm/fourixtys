#pragma once

#include <array>
#include <string_view>
#include "common/bits.h"
#include "common/defines.h"
#include "common/types.h"

using namespace std::string_view_literals;

class N64;

class VR4300 {
public:
    explicit VR4300(N64& system);

    void step();

private:
    N64& m_system;

    bool m_enable_trace_logging { true };

    std::array<u64, 32> m_gprs {};
    u64 m_hi { 0 };
    u64 m_lo { 0 };

    static constexpr std::array m_reg_names = {
        "zero"sv,
        "at"sv,
        "v0"sv, "v1"sv,
        "a0"sv, "a1"sv, "a2"sv, "a3"sv,
        "t0"sv, "t1"sv, "t2"sv, "t3"sv, "t4"sv, "t5"sv, "t6"sv, "t7"sv,
        "s0"sv, "s1"sv, "s2"sv, "s3"sv, "s4"sv, "s5"sv, "s6"sv, "s7"sv,
        "t8"sv, "t9"sv,
        "k0"sv, "k1"sv,
        "gp"sv,
        "sp"sv,
        "s8"sv,
        "ra"sv,
    };

    static constexpr std::string_view reg_name(std::size_t reg_id) {
        return m_reg_names.at(reg_id);
    }

    u64 m_pc { 0 };
    u64 m_next_pc { 0 };
    bool m_about_to_branch { false };

    void simulate_pif_routine();

    ALWAYS_INLINE static u8 get_rs(const u32 instruction) {
        return Common::bit_range<25, 21>(instruction);
    }

    ALWAYS_INLINE static u8 get_rt(const u32 instruction) {
        return Common::bit_range<20, 16>(instruction);
    }

    ALWAYS_INLINE static u8 get_rd(const u32 instruction) {
        return Common::bit_range<15, 11>(instruction);
    }

    void decode_and_execute_instruction(u32 instruction);
    void decode_and_execute_special_instruction(u32 instruction);

    void and_(u32 instruction);
    void andi(u32 instruction);
    void beq(u32 instruction);
    void bne(u32 instruction);
    void jr(u32 instruction);
    void ld(u32 instruction);
    void lui(u32 instruction);
    void lw(u32 instruction);
    void ori(u32 instruction);
    void nop(u32 instruction);
    void sll(u32 instruction);
    void sw(u32 instruction);
};

#pragma once

#include <array>
#include <string_view>
#include "common/bits.h"
#include "common/types.h"

using namespace std::string_view_literals;

class N64;

class RSP {
public:
    RSP(N64& system);

    void step();

    u16 pc() const { return m_pc; }
    void set_pc(u16 pc) {
        m_pc = (pc & ~0b11) & 0xFFF;
        m_next_pc = (m_pc + 4) & 0xFFF;
    }

    bool halted() const { return m_status.flags.halted; }

    u32 status() const { return m_status.raw; }
    void set_status(u32 status);

private:
    N64& m_system;

    u16 m_pc;
    u16 m_next_pc;
    bool m_about_to_branch { false };
    bool m_entering_delay_slot { false };
    bool m_in_delay_slot { false };
    std::array<u32, 32> m_gprs {};

    union {
        u32 raw {};
        struct {
            bool halted : 1;
            bool broke : 1;
            bool dma_busy : 1;
            bool dma_full : 1;
            bool io_busy : 1;
            bool sstep : 1;
            bool intbreak : 1;
            u32 sig : 8;
            u32 : 17;
        } flags;
    } m_status;

    static constexpr std::array m_reg_names = {
        "r0"sv,
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

    ALWAYS_INLINE u32 get_rd(const u32 instruction) const {
        return Common::bit_range<15, 11>(instruction);
    }

    u32 get_current_instruction() const;

    void execute_instruction(u32 instruction);
    void execute_special_instruction(u32 instruction);
    void execute_regimm_instruction(u32 instruction);

    void add(u32 instruction);
    void addi(u32 instruction);
    void addiu(u32 instruction);
    void addu(u32 instruction);
    void and_(u32 instruction);
    void andi(u32 instruction);
    void beq(u32 instruction);
    void bgez(u32 instruction);
    void bgezal(u32 instruction);
    void bgtz(u32 instruction);
    void blez(u32 instruction);
    void bltz(u32 instruction);
    void bltzal(u32 instruction);
    void bne(u32 instruction);
    void break_(u32 instruction);
    void j(u32 instruction);
    void jal(u32 instruction);
    void jalr(u32 instruction);
    void jr(u32 instruction);
    void lui(u32 instruction);
    void nor(u32 instruction);
    void or_(u32 instruction);
    void ori(u32 instruction);
    void sll(u32 instruction);
    void sllv(u32 instruction);
    void sra(u32 instruction);
    void srav(u32 instruction);
    void srl(u32 instruction);
    void srlv(u32 instruction);
    void sub(u32 instruction);
    void subu(u32 instruction);
    void sw(u32 instruction);
    void xor_(u32 instruction);
    void xori(u32 instruction);
};

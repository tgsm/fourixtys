#include "common/bits.h"
#include "common/logging.h"
#include "n64.h"
#include "rsp.h"

#define LTRACE_RSP(disasm_fmt, ...) fmt::print("trace:  [RSP] {:03X}: {:08X}  " disasm_fmt "\n", m_pc, instruction, ##__VA_ARGS__)

RSP::RSP(N64& system) : m_system(system) {
    m_status.flags.halted = true;
}

u32 RSP::get_current_instruction() const {
    return m_system.mmu().read32(0x04001000 | m_pc);
}

void RSP::step() {
    m_gprs[0] = 0;

    const u32 instruction = get_current_instruction();
    execute_instruction(instruction);

    m_pc += 4;
    m_pc &= 0xFFF;
}

void RSP::execute_instruction(const u32 instruction) {
    const auto op = Common::bit_range<31, 26>(instruction);

    switch (op) {
        case 0b000000:
            execute_special_instruction(instruction);
            return;

        case 0b001101:
            ori(instruction);
            return;

        case 0b001111:
            lui(instruction);
            return;

        case 0b101011:
            sw(instruction);
            return;

        default:
            UNIMPLEMENTED_MSG("Unrecognized RSP op {:06b} (instr={:08X}, pc={:03X})", op, instruction, m_pc);
    }
}

void RSP::execute_special_instruction(const u32 instruction) {
    const auto op = Common::bit_range<5, 0>(instruction);

    switch (op) {
        case 0b000000:
            sll(instruction);
            return;

        case 0b001101:
            break_(instruction);
            return;

        case 0b100000:
            add(instruction);
            return;

        default:
            UNIMPLEMENTED_MSG("Unrecognized RSP SPECIAL op {:06b} (instr={:08X}, pc={:03X})", op, instruction, m_pc);
    }
}

void RSP::set_status(const u32 status) {
    LINFO("Setting RSP status {:08X} {:08X}", status, m_system.vr4300().pc());
    if (Common::is_bit_enabled<0>(status)) {
        m_status.flags.halted = false;
    }
    if (Common::is_bit_enabled<1>(status)) {
        m_status.flags.halted = true;
    }
    if (Common::is_bit_enabled<2>(status)) {
        m_status.flags.broke = false;
    }
}

void RSP::add(const u32 instruction) {
    const u16 rs = Common::bit_range<25, 21>(instruction);
    const u16 rt = Common::bit_range<20, 16>(instruction);
    const u16 rd = get_rd(instruction);
    LTRACE_RSP("add ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] + m_gprs[rt];
}

void RSP::break_(const u32 instruction) {
    LTRACE_RSP("break");

    m_status.flags.halted = true;
    m_status.flags.broke = true;
}

void RSP::lui(const u32 instruction) {
    const u16 rt = Common::bit_range<20, 16>(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("lui ${}, 0x{:04X}", reg_name(rt), imm);

    m_gprs[rt] = imm << 16;
}

void RSP::ori(const u32 instruction) {
    const u16 rs = Common::bit_range<25, 21>(instruction);
    const u16 rt = Common::bit_range<20, 16>(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("ori ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] | imm;
}

void RSP::sll(const u32 instruction) {
    if (instruction == 0) {
        LTRACE_RSP("nop");
        return;
    }

    const u16 rt = Common::bit_range<20, 16>(instruction);
    const u16 rd = get_rd(instruction);
    const u16 sa = Common::bit_range<10, 6>(instruction);
    LTRACE_RSP("sll ${}, ${}, {}", reg_name(rt), reg_name(rd), sa);
    // FIXME: Figure out format for this trace.

    UNIMPLEMENTED();
}

void RSP::sw(const u32 instruction) {
    const u16 rs = Common::bit_range<25, 21>(instruction);
    const u16 rt = Common::bit_range<20, 16>(instruction);
    const s16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("sw ${}, 0x{:04X}(${})", reg_name(rt), imm, reg_name(rs));

    LINFO("Writing {:08X} to {:08X}", m_gprs[rt], 0x04000000 | (m_gprs[rs] + imm));
    m_system.mmu().write32(0x04000000 | (m_gprs[rs] + imm), m_gprs[rt]);
}

#include <fmt/core.h>
#include "vr4300.h"
#include "n64.h"

VR4300::VR4300(N64& system) : m_system(system) {
    simulate_pif_routine();
}

void VR4300::simulate_pif_routine() {
    // copied from MAME
    m_gprs[ 1] = 0x0000000000000001;
    m_gprs[ 2] = 0x000000000EBDA536;
    m_gprs[ 3] = 0x000000000EBDA536;
    m_gprs[ 4] = 0x000000000000A536;
    m_gprs[ 5] = 0xFFFFFFFFC0F1D859;
    m_gprs[ 6] = 0xFFFFFFFFA4001F0C;
    m_gprs[ 7] = 0xFFFFFFFFA4001F08;
    m_gprs[ 8] = 0x00000000000000C0;
    m_gprs[ 9] = 0x0000000000000000;
    m_gprs[10] = 0x0000000000000040;
    m_gprs[11] = 0xFFFFFFFFA4000040;
    m_gprs[12] = 0xFFFFFFFFED10D0B3;
    m_gprs[13] = 0x000000001402A4CC;
    m_gprs[14] = 0x000000002DE108EA;
    m_gprs[15] = 0x000000003103E121;
    m_gprs[16] = 0x0000000000000000;
    m_gprs[17] = 0x0000000000000000;
    m_gprs[18] = 0x0000000000000000;
    m_gprs[19] = 0x0000000000000000;
    m_gprs[20] = 0x0000000000000001;
    m_gprs[21] = 0x0000000000000000;
    m_gprs[22] = 0x000000000000003F;
    m_gprs[23] = 0x0000000000000000;
    m_gprs[24] = 0x0000000000000000;
    m_gprs[25] = 0xFFFFFFFF9DEBB54F;
    m_gprs[26] = 0x0000000000000000;
    m_gprs[27] = 0x0000000000000000;
    m_gprs[28] = 0x0000000000000000;
    m_gprs[29] = 0xFFFFFFFFA4001FF0;
    m_gprs[30] = 0x0000000000000000;
    m_gprs[31] = 0xFFFFFFFFA4001550;
    m_hi = 0x000000003FC18657;
    m_lo = 0x000000003103E121;

    const u32 source_address = 0xB0000000;
    const u32 destination_address = 0xA4000000;
    for (u32 i = 0; i < 0x1000; i++) {
        m_system.mmu().write8(destination_address + i, m_system.mmu().read8(source_address + i));
    }

    m_pc = 0xA4000040;
    m_next_pc = m_pc + 4;
}

void VR4300::step() {
    // Always reset the zero register, just in case
    m_gprs[0] = 0;

    const u32 instruction = m_system.mmu().read32(m_pc);
    decode_and_execute_instruction(instruction);

    if (!m_about_to_branch) {
        m_pc = m_next_pc;
        m_next_pc += 4;
    } else {
        m_pc += 4;
        m_about_to_branch = false;
    }
}

void VR4300::decode_and_execute_instruction(u32 instruction) {
    const auto op = Common::bit_range<31, 26>(instruction);

    switch (op) {
        case 0b000000:
            decode_and_execute_special_instruction(instruction);
            return;

        case 0b000001:
            decode_and_execute_regimm_instruction(instruction);
            return;

        case 0b010000:
            decode_and_execute_cop0_instruction(instruction);
            return;

        case 0b000010:
            j(instruction);
            return;

        case 0b000011:
            jal(instruction);
            return;

        case 0b000100:
            beq(instruction);
            return;

        case 0b000101:
            bne(instruction);
            return;

        case 0b001000:
            addi(instruction);
            return;

        case 0b001001:
            addiu(instruction);
            return;

        case 0b001010:
            slti(instruction);
            return;

        case 0b001011:
            sltiu(instruction);
            return;

        case 0b001100:
            andi(instruction);
            return;

        case 0b001101:
            ori(instruction);
            return;

        case 0b001110:
            xori(instruction);
            return;

        case 0b001111:
            lui(instruction);
            return;

        case 0b010100:
            beql(instruction);
            return;

        case 0b010101:
            bnel(instruction);
            return;

        case 0b011000:
            daddi(instruction);
            return;

        case 0b011001:
            daddiu(instruction);
            return;

        case 0b100011:
            lw(instruction);
            return;

        case 0b100100:
            lbu(instruction);
            return;

        case 0b100101:
            lhu(instruction);
            return;

        case 0b100111:
            lwu(instruction);
            return;

        case 0b101011:
            sw(instruction);
            return;

        case 0b101111:
            cache(instruction);
            return;

        case 0b110111:
            ld(instruction);
            return;

        default:
            UNIMPLEMENTED_MSG("Unrecognized VR4300 op {:06b} ({}, {}) (instr={:08X}, pc={:016X})", op, op >> 3, op & 7, instruction, m_pc);
    }
}

void VR4300::decode_and_execute_special_instruction(u32 instruction) {
    const auto op = Common::bit_range<5, 0>(instruction);

    switch (op) {
        case 0b000000:
            sll(instruction);
            return;

        case 0b000010:
            srl(instruction);
            return;

        case 0b000011:
            sra(instruction);
            return;

        case 0b000100:
            sllv(instruction);
            return;

        case 0b000110:
            srlv(instruction);
            return;

        case 0b000111:
            srav(instruction);
            return;

        case 0b001000:
            jr(instruction);
            return;

        case 0b001001:
            jalr(instruction);
            return;

        case 0b010010:
            mflo(instruction);
            return;

        case 0b010100:
            dsllv(instruction);
            return;

        case 0b011001:
            multu(instruction);
            return;

        case 0b100000:
            add(instruction);
            return;

        case 0b100001:
            addu(instruction);
            return;

        case 0b100011:
            subu(instruction);
            return;

        case 0b100100:
            and_(instruction);
            return;

        case 0b100101:
            or_(instruction);
            return;

        case 0b100110:
            xor_(instruction);
            return;

        case 0b100111:
            nor(instruction);
            return;

        case 0b101010:
            slt(instruction);
            return;

        case 0b101011:
            sltu(instruction);
            return;

        case 0b111000:
            dsll(instruction);
            return;

        case 0b111100:
            dsll32(instruction);
            return;

        case 0b111111:
            dsra32(instruction);
            return;

        default:
            UNIMPLEMENTED_MSG("unrecognized VR4300 SPECIAL op {:06b} ({}, {}) (instr={:08X}, pc={:016X})", op, op >> 3, op & 7, instruction, m_pc);
    }
}

void VR4300::decode_and_execute_regimm_instruction(u32 instruction) {
    const auto op = Common::bit_range<20, 16>(instruction);

    switch (op) {
        case 0b10001:
            bgezal(instruction);
            return;

        default:
            UNIMPLEMENTED_MSG("unrecognized VR4300 REGIMM op {:05b} ({}, {}) (instr={:08X}, pc={:016X})", op, op >> 3, op & 7, instruction, m_pc);
    }
}

void VR4300::decode_and_execute_cop0_instruction(u32 instruction) {
    const auto op = Common::bit_range<25, 21>(instruction);

    switch (op) {
        case 0b00100:
            mtc0(instruction);
            return;

        default:
            UNIMPLEMENTED_MSG("unrecognized COP0 op {:05b} (instr={:08X}, pc={:016X})", op, instruction, m_pc);
    }
}

void VR4300::add(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("add ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] + m_gprs[rt];
}

void VR4300::addi(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("addi ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] + s16(imm);
}

void VR4300::addiu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("addiu ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = static_cast<s32>(m_gprs[rs] + s16(imm));
}

void VR4300::addu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("addu ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = static_cast<s32>(m_gprs[rs] + m_gprs[rt]);
}

void VR4300::and_(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("and ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] & m_gprs[rt];
}

void VR4300::andi(u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("andi ${}, ${}, 0x{:04X}", reg_name(rs), reg_name(rt), imm);

    m_gprs[rt] = m_gprs[rs] & imm;
}

void VR4300::beq(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    [[maybe_unused]] const s16 offset = Common::bit_range<15, 0>(instruction);
    const u64 new_pc = m_pc + 4 + (offset << 2);
    LTRACE_VR4300("beq ${}, ${}, 0x{:04X}", reg_name(rs), reg_name(rt), new_pc);

    if (m_gprs[rs] == m_gprs[rt]) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }
}

void VR4300::beql(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    [[maybe_unused]] const s16 offset = Common::bit_range<15, 0>(instruction);
    const u64 new_pc = m_pc + 4 + (offset << 2);
    LTRACE_VR4300("beql ${}, ${}, 0x{:04X}", reg_name(rs), reg_name(rt), new_pc);

    if (m_gprs[rs] == m_gprs[rt]) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    } else {
        m_next_pc += 4;
    }
}

void VR4300::bgezal(const u32 instruction) {
    const auto rs = get_rs(instruction);
    [[maybe_unused]] const s16 offset = Common::bit_range<15, 0>(instruction);
    const u64 new_pc = m_pc + 4 + (offset << 2);
    LTRACE_VR4300("bgezal ${}, 0x{:04X}", reg_name(rs), new_pc);

    m_gprs[31] = m_pc + 8;

    if (static_cast<s64>(m_gprs[rs]) >= 0) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }
}

void VR4300::bne(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    [[maybe_unused]] const s16 offset = Common::bit_range<15, 0>(instruction);
    const u64 new_pc = m_pc + 4 + (offset << 2);
    LTRACE_VR4300("bne ${}, ${}, 0x{:04X}", reg_name(rs), reg_name(rt), new_pc);

    if (m_gprs[rs] != m_gprs[rt]) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }
}

void VR4300::bnel(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    [[maybe_unused]] const s16 offset = Common::bit_range<15, 0>(instruction);
    const u64 new_pc = m_pc + 4 + (offset << 2);
    LTRACE_VR4300("bnel ${}, ${}, 0x{:04X}", reg_name(rs), reg_name(rt), new_pc);

    if (m_gprs[rs] != m_gprs[rt]) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    } else {
        m_next_pc += 4;
    }
}

void VR4300::cache(const u32 instruction) {
    const auto base = Common::bit_range<25, 21>(instruction);
    const auto op = Common::bit_range<20, 16>(instruction);
    const s16 offset = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("cache {}, 0x{:04X}(${})", op, offset, reg_name(base));

    LWARN("CACHE is stubbed");
}

void VR4300::daddi(const u32 instruction) {
    // FIXME: An integer overflow exception occurs if carries out of
    //        bits 62 and 63 differ (2’s complement overflow).  The
    //        contents of destination register rt is not modified when
    //        an integer overflow exception occurs.

    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("daddi ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    const s64 addend1 = m_gprs[rs];
    const s16 addend2 = imm;
    const s64 result = addend1 + addend2;

    m_gprs[rt] = result;
}

void VR4300::daddiu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("daddiu ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] + s16(imm);
}

void VR4300::dsll(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const auto sa = Common::bit_range<10, 6>(instruction);
    LTRACE_VR4300("dsll ${}, ${}, ${}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = m_gprs[rt] << sa;
}

void VR4300::dsllv(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("dsllv ${}, ${}, ${}", reg_name(rd), reg_name(rt), reg_name(rs));

    m_gprs[rd] = m_gprs[rt] << Common::lowest_bits(m_gprs[rs], 6);
}

void VR4300::dsll32(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const auto sa = Common::bit_range<10, 6>(instruction);
    LTRACE_VR4300("dsll32 ${}, ${}, ${}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = m_gprs[rt] << (32 + sa);
}

void VR4300::dsra32(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const auto sa = Common::bit_range<10, 6>(instruction);
    LTRACE_VR4300("dsra32 ${}, ${}, ${}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = static_cast<s64>(m_gprs[rt]) >> (32 + sa);
}

void VR4300::j(const u32 instruction) {
    const auto target = Common::bit_range<25, 0>(instruction);
    const u32 destination = (m_pc & 0xF0000000) | (target << 2);
    LTRACE_VR4300("j 0x{:08X}", destination);

    m_next_pc = destination;
    m_about_to_branch = true;
}

void VR4300::jal(const u32 instruction) {
    const auto target = Common::bit_range<25, 0>(instruction);
    const u32 destination = (m_pc & 0xF0000000) | (target << 2);
    LTRACE_VR4300("jal 0x{:08X}", destination);

    m_gprs[31] = m_pc + 8;

    m_next_pc = destination;
    m_about_to_branch = true;
}

void VR4300::jalr(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("jalr ${}, ${}", reg_name(rd), reg_name(rs));

    m_gprs[rd] = m_pc + 8;

    m_next_pc = m_gprs[rs];
    m_about_to_branch = true;
}

void VR4300::jr(const u32 instruction) {
    // FIXME: If these low-order two bits are not zero, an address exception will occur when the jump target instruction is fetched.

    const auto rs = get_rs(instruction);
    LTRACE_VR4300("jr ${}", reg_name(rs));

    m_next_pc = m_gprs[rs];
    m_about_to_branch = true;
}

void VR4300::lbu(const u32 instruction) {
    // TODO: exceptions

    const auto base = Common::bit_range<25, 21>(instruction);
    const auto rt = get_rt(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("lbu ${}, 0x{:04X}(${})", reg_name(rt), offset, reg_name(base));

    const u32 address = m_gprs[base] + static_cast<s16>(offset);
    m_gprs[rt] = m_system.mmu().read8(address);
}

void VR4300::ld(const u32 instruction) {
    // FIXME: exceptions

    const auto base = Common::bit_range<25, 21>(instruction);
    const auto rt = get_rt(instruction);
    const s16 offset = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("ld ${}, 0x{:04X}(${})", reg_name(rt), offset, reg_name(base));

    const u32 address = m_gprs[base] + s16(offset);
    m_gprs[rt] = m_system.mmu().read64(address);
}

void VR4300::lhu(const u32 instruction) {
    // TODO: exceptions

    const auto base = Common::bit_range<25, 21>(instruction);
    const auto rt = get_rt(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("lhu ${}, 0x{:04X}(${})", reg_name(rt), offset, reg_name(base));

    const u32 address = m_gprs[base] + static_cast<s16>(offset);
    m_gprs[rt] = m_system.mmu().read16(address);
}

void VR4300::lui(u32 instruction) {
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("lui ${}, 0x{:04X}", reg_name(rt), imm);

    m_gprs[rt] = static_cast<s32>(imm << 16);
}

void VR4300::lw(const u32 instruction) {
    // FIXME: throw an exception if either of the low-order two bits of the address is not zero

    const auto base = Common::bit_range<25, 21>(instruction);
    const auto rt = get_rt(instruction);
    const s16 offset = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("lw ${}, 0x{:04X}(${})", reg_name(rt), offset, reg_name(base));

    const u32 address = m_gprs[base] + s16(offset);
    m_gprs[rt] = static_cast<s32>(m_system.mmu().read32(address));
}

void VR4300::lwu(const u32 instruction) {
    // TODO: throw an exception if either of the low-order two bits of the address is not zero

    const auto base = Common::bit_range<25, 21>(instruction);
    const auto rt = get_rt(instruction);
    const s16 offset = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("lwu ${}, 0x{:04X}(${})", reg_name(rt), offset, reg_name(base));

    const u32 address = m_gprs[base] + s16(offset);
    m_gprs[rt] = m_system.mmu().read32(address);
}

void VR4300::mflo(const u32 instruction) {
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("mflo ${}", reg_name(rd));

    m_gprs[rd] = m_lo;
}

void VR4300::mtc0(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("mtc0 ${}, ${}", reg_name(rt), rd);

    LWARN("MTC0 is stubbed");
}

void VR4300::multu(const u32 instruction) {
    // FIXME: edge cases

    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    LTRACE_VR4300("multu ${}, ${}", reg_name(rs), reg_name(rt));

    const u64 result = m_gprs[rs] * m_gprs[rt];
    m_hi = static_cast<s32>(Common::bit_range<63, 32>(result));
    m_lo = static_cast<s32>(Common::bit_range<31, 0>(result));
}

void VR4300::nop(const u32 instruction) {
    LTRACE_VR4300("nop");
}

void VR4300::nor(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("nor ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = ~(m_gprs[rs] | m_gprs[rt]);
}

void VR4300::or_(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("or ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] | m_gprs[rt];
}

void VR4300::ori(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("ori ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] | imm;
}

void VR4300::sll(const u32 instruction) {
    if (instruction == 0) {
        nop(instruction);
        return;
    }

    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const auto sa = Common::bit_range<10, 6>(instruction);
    LTRACE_VR4300("sll ${}, ${}, {}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = static_cast<s32>(m_gprs[rt] << sa);
}

void VR4300::sllv(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("sllv ${}, ${}, {}", reg_name(rd), reg_name(rt), reg_name(rs));

    m_gprs[rd] = static_cast<s32>(m_gprs[rt] << Common::lowest_bits(m_gprs[rs], 5));
}

void VR4300::slt(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("slt ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = (s64(m_gprs[rs]) < s64(m_gprs[rt]));
}

void VR4300::slti(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const s16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("slti ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = (static_cast<s64>(m_gprs[rs]) < imm);
}

void VR4300::sltiu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const s16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("sltiu ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = ((m_gprs[rs] - imm) < static_cast<u64>(static_cast<s64>(imm)));
}

void VR4300::sltu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("sltu ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = (m_gprs[rs] < m_gprs[rt]);
}

void VR4300::sra(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const auto sa = Common::bit_range<10, 6>(instruction);
    LTRACE_VR4300("sra ${}, ${}, ${}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = static_cast<s32>(m_gprs[rt] >> sa);
}

void VR4300::srav(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("srav ${}, ${}, {}", reg_name(rd), reg_name(rt), reg_name(rs));

    m_gprs[rd] = static_cast<s32>(m_gprs[rt] >> Common::lowest_bits(m_gprs[rs], 5));
}

void VR4300::srl(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const auto sa = Common::bit_range<10, 6>(instruction);
    LTRACE_VR4300("srl ${}, ${}, ${}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = static_cast<s32>(static_cast<u32>(m_gprs[rt]) >> sa);
}

void VR4300::srlv(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("srlv ${}, ${}, {}", reg_name(rd), reg_name(rt), reg_name(rs));

    m_gprs[rd] = static_cast<s32>(static_cast<u32>(m_gprs[rt]) >> Common::lowest_bits(m_gprs[rs], 5));
}

void VR4300::subu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("subu ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = static_cast<s32>(m_gprs[rs] - m_gprs[rt]);
}

void VR4300::sw(const u32 instruction) {
    // FIXME: If either of the low-order two bits of the address are not zero, an address error exception occurs.

    const auto base = Common::bit_range<25, 21>(instruction);
    const auto rt = get_rt(instruction);
    const s16 offset = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("sw ${}, 0x{:04X}(${})", reg_name(rt), offset, reg_name(base));

    const u32 address = m_gprs[base] + static_cast<s16>(offset);
    m_system.mmu().write32(address, m_gprs[rt]);
}

void VR4300::xor_(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_VR4300("xor ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] ^ m_gprs[rt];
}

void VR4300::xori(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_VR4300("xori ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] ^ imm;
}

#include "common/bits.h"
#include "common/logging.h"
#include "n64.h"
#include "rsp.h"

#define LTRACE_RSP(disasm_fmt, ...) // fmt::print("trace:  [RSP] {:03X}: {:08X}  " disasm_fmt "\n", m_pc, instruction, ##__VA_ARGS__)

RSP::RSP(N64& system) : m_system(system) {
    m_status.flags.halted = true;

    m_pc = 0;
    m_next_pc = m_pc + 4;
}

u32 RSP::get_current_instruction() const {
    return m_system.mmu().read32(0x04001000 | m_pc);
}

void RSP::step() {
    m_gprs[0] = 0;

    const u32 instruction = get_current_instruction();
    execute_instruction(instruction);

    if (m_in_delay_slot) {
        m_in_delay_slot = false;
    }

    if (m_entering_delay_slot) {
        m_in_delay_slot = true;
        m_entering_delay_slot = false;
    }

    if (!m_about_to_branch) {
        m_pc = m_next_pc & 0xFFF;
        m_next_pc += 4;
        m_next_pc &= 0xFFF;
    } else {
        m_pc += 4;
        m_pc &= 0xFFF;
        m_about_to_branch = false;
    }
}

void RSP::set_status(const u32 status) {
    // LINFO("Setting RSP status {:08X} {:08X}", status, m_system.vr4300().pc());
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

void RSP::execute_instruction(const u32 instruction) {
    const auto op = Common::bit_range<31, 26>(instruction);

    switch (op) {
        case 0b000000:
            execute_special_instruction(instruction);
            return;

        case 0b000001:
            execute_regimm_instruction(instruction);
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

        case 0b000110:
            blez(instruction);
            return;

        case 0b000111:
            bgtz(instruction);
            return;

        case 0b001000:
            addi(instruction);
            return;

        case 0b001001:
            addiu(instruction);
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

        case 0b001101:
            break_(instruction);
            return;

        case 0b100000:
            add(instruction);
            return;

        case 0b100001:
            addu(instruction);
            return;

        case 0b100010:
            sub(instruction);
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

        default:
            UNIMPLEMENTED_MSG("Unrecognized RSP SPECIAL op {:06b} (instr={:08X}, pc={:03X})", op, instruction, m_pc);
    }
}

void RSP::execute_regimm_instruction(const u32 instruction) {
    const auto op = Common::bit_range<20, 16>(instruction);

    switch (op) {
        case 0b000000:
            bltz(instruction);
            return;

        case 0b000001:
            bgez(instruction);
            return;

        case 0b010000:
            bltzal(instruction);
            return;

        case 0b010001:
            bgezal(instruction);
            return;

        default:
            UNIMPLEMENTED_MSG("Unrecognized RSP REGIMM op {:06b} (instr={:08X}, pc={:03X})", op, instruction, m_pc);
    }
}

void RSP::add(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("add ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));
    // FIXME: Does this throw an exception on overflow?

    m_gprs[rd] = m_gprs[rs] + m_gprs[rt];
}

void RSP::addi(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const s16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("addi ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);
    // FIXME: Does this throw an exception on overflow?

    m_gprs[rt] = m_gprs[rs] + imm;
}

void RSP::addiu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const s16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("addiu ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] + imm;
}

void RSP::addu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("addu ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] + m_gprs[rt];
}

void RSP::and_(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("and ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] & m_gprs[rt];
}

void RSP::andi(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("andi ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] & imm;
}

void RSP::beq(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("beq ${}, ${}, 0x{:03X}", reg_name(rs), reg_name(rt), new_pc);

    if (m_gprs[rs] == m_gprs[rt]) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
}

void RSP::bgez(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("bgez ${}, 0x{:03X}", reg_name(rs), new_pc);

    if (static_cast<s32>(m_gprs[rs]) >= 0) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
}

void RSP::bgezal(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("bgezal ${}, 0x{:03X}", reg_name(rs), new_pc);

    if (static_cast<s32>(m_gprs[rs]) >= 0) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
    m_gprs[31] = (m_pc + 8) & 0xFFF;
}

void RSP::bgtz(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("bgtz ${}, 0x{:03X}", reg_name(rs), new_pc);

    if (static_cast<s32>(m_gprs[rs]) > 0) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
}

void RSP::blez(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("blez ${}, 0x{:03X}", reg_name(rs), new_pc);

    if (static_cast<s32>(m_gprs[rs]) <= 0) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
}

void RSP::bltz(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("bltz ${}, 0x{:03X}", reg_name(rs), new_pc);

    if (static_cast<s32>(m_gprs[rs]) < 0) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
}

void RSP::bltzal(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("bltzal ${}, 0x{:03X}", reg_name(rs), new_pc);

    if (static_cast<s32>(m_gprs[rs]) < 0) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
    m_gprs[31] = (m_pc + 8) & 0xFFF;
}

void RSP::bne(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 offset = Common::bit_range<15, 0>(instruction);
    const u16 new_pc = (m_pc + 4 + (offset << 2)) & 0xFFF;
    LTRACE_RSP("bne ${}, ${}, 0x{:03X}", reg_name(rs), reg_name(rt), new_pc);

    if (m_gprs[rs] != m_gprs[rt]) {
        m_next_pc = new_pc;
        m_about_to_branch = true;
    }

    m_entering_delay_slot = true;
}

void RSP::break_(const u32 instruction) {
    LTRACE_RSP("break");

    m_status.flags.halted = true;
    m_status.flags.broke = true;

    m_about_to_branch = false;
    m_entering_delay_slot = false;
    m_in_delay_slot = false;
}

void RSP::j(const u32 instruction) {
    const auto target = Common::bit_range<25, 0>(instruction);
    const u16 new_pc = (target << 2) & 0xFFF;
    LTRACE_RSP("j 0x{:03X}", new_pc);

    m_next_pc = new_pc;
    m_about_to_branch = true;
    m_entering_delay_slot = true;
}

void RSP::jal(const u32 instruction) {
    const auto target = Common::bit_range<25, 0>(instruction);
    const u16 new_pc = (target << 2) & 0xFFF;
    LTRACE_RSP("jal 0x{:03X}", new_pc);

    m_gprs[31] = (m_pc + 8) & 0xFFF;

    m_next_pc = new_pc;
    m_about_to_branch = true;
    m_entering_delay_slot = true;
}

void RSP::jalr(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("jalr ${}, ${}", reg_name(rd), reg_name(rs));

    m_next_pc = (m_gprs[rs] & ~0b11) & 0xFFF;
    m_gprs[rd] = (m_pc + 8) & 0xFFF;
    m_about_to_branch = true;
    m_entering_delay_slot = true;
}

void RSP::jr(const u32 instruction) {
    const auto rs = get_rs(instruction);
    LTRACE_RSP("jr ${}", reg_name(rs));

    m_next_pc = (m_gprs[rs] & ~0b11) & 0xFFF;
    m_about_to_branch = true;
    m_entering_delay_slot = true;
}

void RSP::lui(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("lui ${}, 0x{:04X}", reg_name(rt), imm);

    m_gprs[rt] = imm << 16;
}

void RSP::nor(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("nor ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = ~(m_gprs[rs] | m_gprs[rt]);
}

void RSP::or_(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("or ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] | m_gprs[rt];
}

void RSP::ori(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("ori ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] | imm;
}

void RSP::sll(const u32 instruction) {
    if (instruction == 0) {
        LTRACE_RSP("nop");
        return;
    }

    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const u16 sa = Common::bit_range<10, 6>(instruction);
    LTRACE_RSP("sll ${}, ${}, {}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = m_gprs[rt] << sa;
}

void RSP::sllv(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("sllv ${}, ${}, ${}", reg_name(rd), reg_name(rt), reg_name(rs));

    m_gprs[rd] = m_gprs[rt] << (m_gprs[rs] & 0x1F);
}

void RSP::sra(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const u16 sa = Common::bit_range<10, 6>(instruction);
    LTRACE_RSP("sra ${}, ${}, {}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = static_cast<s32>(m_gprs[rt]) >> sa;
}

void RSP::srav(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("srav ${}, ${}, ${}", reg_name(rd), reg_name(rt), reg_name(rs));

    m_gprs[rd] = static_cast<s32>(m_gprs[rt]) >> (m_gprs[rs] & 0x1F);
}

void RSP::srl(const u32 instruction) {
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    const u16 sa = Common::bit_range<10, 6>(instruction);
    LTRACE_RSP("srl ${}, ${}, {}", reg_name(rd), reg_name(rt), sa);

    m_gprs[rd] = m_gprs[rt] >> sa;
}

void RSP::srlv(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("srlv ${}, ${}, ${}", reg_name(rd), reg_name(rt), reg_name(rs));

    m_gprs[rd] = m_gprs[rt] >> (m_gprs[rs] & 0x1F);
}

void RSP::sub(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("sub ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));
    // FIXME: Does this throw an exception on underflow?

    m_gprs[rd] = m_gprs[rs] - m_gprs[rt];
}

void RSP::subu(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("subu ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] - m_gprs[rt];
}

void RSP::sw(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const s16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("sw ${}, 0x{:04X}(${})", reg_name(rt), imm, reg_name(rs));

    // LINFO("Writing {:08X} to {:08X}", m_gprs[rt], 0x04000000 | (m_gprs[rs] + imm));
    m_system.mmu().write32(0x04000000 | (m_gprs[rs] + imm), m_gprs[rt]);
}

void RSP::xor_(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const auto rd = get_rd(instruction);
    LTRACE_RSP("xor ${}, ${}, ${}", reg_name(rd), reg_name(rs), reg_name(rt));

    m_gprs[rd] = m_gprs[rs] ^ m_gprs[rt];
}

void RSP::xori(const u32 instruction) {
    const auto rs = get_rs(instruction);
    const auto rt = get_rt(instruction);
    const u16 imm = Common::bit_range<15, 0>(instruction);
    LTRACE_RSP("xori ${}, ${}, 0x{:04X}", reg_name(rt), reg_name(rs), imm);

    m_gprs[rt] = m_gprs[rs] ^ imm;
}

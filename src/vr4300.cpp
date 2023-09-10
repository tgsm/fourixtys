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
    // decode_and_execute_instruction(instruction);

    fmt::print("instruction: {:08X}\n", instruction);
}

#include "common/logging.h"
#include "mmu.h"
#include "n64.h"

static constexpr u32 KUSEG_BASE = 0x00000000;
static constexpr u32 KSEG0_BASE = 0x80000000;
static constexpr u32 KSEG1_BASE = 0xA0000000;
static constexpr u32 KSSEG_BASE = 0xC0000000;
static constexpr u32 KSEG3_BASE = 0xE0000000;

static constexpr u32 RDRAM_BUILTIN_BASE        = 0x00000000;
static constexpr u32 RDRAM_BUILTIN_END         = 0x003FFFFF;

static constexpr u32 RDRAM_EXPANSION_BASE      = 0x00400000;
static constexpr u32 RDRAM_EXPANSION_END       = 0x007FFFFF;

static constexpr u32 RDRAM_REGISTERS_BASE      = 0x03F00000;
static constexpr u32 RDRAM_REGISTERS_END       = 0x03FFFFFF;

static constexpr u32 SP_DMEM_BASE              = 0x04000000;
static constexpr u32 SP_DMEM_END               = 0x04000FFF;

static constexpr u32 SP_IMEM_BASE              = 0x04001000;
static constexpr u32 SP_IMEM_END               = 0x04001FFF;

static constexpr u32 SP_REGISTERS_BASE         = 0x04040000;
static constexpr u32 SP_REGISTERS_END          = 0x040FFFFF;

static constexpr u32 DP_COMMAND_REGISTERS_BASE = 0x04100000;
static constexpr u32 DP_COMMAND_REGISTERS_END  = 0x041FFFFF;

static constexpr u32 DP_SPAN_REGISTERS_BASE    = 0x04200000;
static constexpr u32 DP_SPAN_REGISTERS_END     = 0x042FFFFF;

static constexpr u32 MI_REGISTERS_BASE         = 0x04300000;
static constexpr u32 MI_REG_MODE               = 0x04300000;
static constexpr u32 MI_REG_VERSION            = 0x04300004;
static constexpr u32 MI_REG_INTR               = 0x04300008;
static constexpr u32 MI_REG_INTR_MASK          = 0x0430000C;
static constexpr u32 MI_REGISTERS_END          = 0x043FFFFF;

static constexpr u32 VI_REGISTERS_BASE         = 0x04400000;
static constexpr u32 VI_REG_CONTROL            = 0x04400000;
static constexpr u32 VI_REG_ORIGIN             = 0x04400004;
static constexpr u32 VI_REG_WIDTH              = 0x04400008;
static constexpr u32 VI_REG_V_INTR             = 0x0440000C;
static constexpr u32 VI_REG_V_CURRENT          = 0x04400010;
static constexpr u32 VI_REG_BURST              = 0x04400014;
static constexpr u32 VI_REG_V_SYNC             = 0x04400018;
static constexpr u32 VI_REG_H_SYNC             = 0x0440001C;
static constexpr u32 VI_REG_LEAP               = 0x04400020;
static constexpr u32 VI_REG_H_START            = 0x04400024;
static constexpr u32 VI_REG_V_START            = 0x04400028;
static constexpr u32 VI_REG_V_BURST            = 0x0440002C;
static constexpr u32 VI_REG_X_SCALE            = 0x04400030;
static constexpr u32 VI_REG_Y_SCALE            = 0x04400034;
static constexpr u32 VI_REGISTERS_END          = 0x044FFFFF;

static constexpr u32 AI_REGISTERS_BASE         = 0x04500000;
static constexpr u32 AI_REGISTERS_END          = 0x045FFFFF;

static constexpr u32 PI_REGISTERS_BASE         = 0x04600000;
static constexpr u32 PI_REGISTERS_END          = 0x046FFFFF;

static constexpr u32 RI_REGISTERS_BASE         = 0x04700000;
static constexpr u32 RI_REGISTERS_END          = 0x047FFFFF;

static constexpr u32 SI_REGISTERS_BASE         = 0x04800000;
static constexpr u32 SI_REGISTERS_END          = 0x048FFFFF;

static constexpr u32 PIF_BOOTROM_BASE          = 0x1FC00000;
static constexpr u32 PIF_BOOTROM_END           = 0x1FC007BF;

static constexpr u32 PIF_RAM_BASE              = 0x1FC007C0;
static constexpr u32 PIF_RAM_END               = 0x1FC007FF;

constexpr MMU::AddressRanges MMU::address_range(const u32 virtual_address) {
    if (virtual_address < KSEG0_BASE) {
        return AddressRanges::User;
    }

    if (virtual_address < KSEG1_BASE) {
        return AddressRanges::Kernel0;
    }

    if (virtual_address < KSSEG_BASE) {
        return AddressRanges::Kernel1;
    }

    if (virtual_address < KSEG3_BASE) {
        return AddressRanges::KernelSupervisor;
    }

    return AddressRanges::Kernel3;
}

constexpr u32 MMU::virtual_address_to_physical_address(const u32 virtual_address) {
    const AddressRanges range = address_range(virtual_address);
    switch (range) {
        case AddressRanges::User:
            return virtual_address - KUSEG_BASE;
        case AddressRanges::Kernel0:
            return virtual_address - KSEG0_BASE;
        case AddressRanges::Kernel1:
            return virtual_address - KSEG1_BASE;
        case AddressRanges::KernelSupervisor:
            return virtual_address - KSSEG_BASE;
        case AddressRanges::Kernel3:
            return virtual_address - KSEG3_BASE;
        default:
            UNREACHABLE_MSG("Unreachable address range {}", Common::underlying(range));
    }
}

template <typename T>
T MMU::read(u32 address) {
    switch (address) {
        case SP_DMEM_BASE ... SP_DMEM_END:
            if constexpr (Common::TypeIsSame<T, u32>) {
                const u32 idx = address - SP_DMEM_BASE;
                return m_sp_dmem.at(idx + 0) << 24 |
                       m_sp_dmem.at(idx + 1) << 16 |
                       m_sp_dmem.at(idx + 2) << 8  |
                       m_sp_dmem.at(idx + 3) << 0;
            } else {
                UNIMPLEMENTED_MSG("Unrecognized read{} from SP dmem", Common::TypeSizeInBits<T>);
            }


        case 0x10000000 ... 0x1FBFFFFF:
            return m_system.gamepak().read<T>(address - 0x10000000);

        case 0x80000000 ... 0xFFFFFFFF:
            return read<T>(virtual_address_to_physical_address(address));

        default:
            LERROR("Unrecognized read{} from 0x{:08X}", Common::TypeSizeInBits<T>, address);
            return T(-1);
    }
}

template <typename T>
void MMU::write(u32 address, T value) {
    switch (address) {
        case SP_DMEM_BASE ... SP_DMEM_END:
            if constexpr (Common::TypeIsSame<T, u8>) {
                m_sp_dmem.at(address - SP_DMEM_BASE) = value;
                return;
            } else {
                UNIMPLEMENTED_MSG("Unimplemented write{} 0x{:08X} to SP dmem", Common::TypeSizeInBits<T>, value);
            }

        case 0x80000000 ... 0xFFFFFFFF:
            write<T>(virtual_address_to_physical_address(address), value);
            return;

        default:
            LERROR("Unrecognized write{} 0x{:08X} to 0x{:08X}", Common::TypeSizeInBits<T>, value, address);
            return;
    }    
}

u8 MMU::read8(const u32 address) {
    return read<u8>(address);
}

void MMU::write8(const u32 address, const u8 value) {
    write<u8>(address, value);
}

u16 MMU::read16(const u32 address) {
    return read<u16>(address);
}

void MMU::write16(const u32 address, const u16 value) {
    write<u16>(address, value);
}

u32 MMU::read32(const u32 address) {
    return read<u32>(address);
}

void MMU::write32(const u32 address, const u32 value) {
    write<u32>(address, value);
}

u64 MMU::read64(const u32 address) {
    return read<u64>(address);
}

void MMU::write64(const u32 address, const u64 value) {
    write<u64>(address, value);
}

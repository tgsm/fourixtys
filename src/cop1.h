#pragma once

#include <array>
#include <bit>
#include <string_view>
#include "common/bits.h"
#include "common/defines.h"
#include "common/types.h"

class VR4300;

class COP1 {
public:
    explicit COP1(VR4300& vr4300) : m_vr4300(vr4300) {}

    static std::string_view reg_name(u32 reg);

    enum class Conditions {
        UN,
        EQ,
        UEQ,
        OLT,
        ULT,
        OLE,
        ULE,
        SF,
        NGLE,
        SEQ,
        NGL,
        LT,
        NGE,
        LE,
        NGT,
    };
    static std::string_view condition_name(Conditions condition);

    enum Format {
        SingleFloating = 16,
        S = SingleFloating,

        DoubleFloating = 17,
        D = DoubleFloating,

        Word = 20,
        W = Word,

        Long = 21,
        L = Long,
    };
    static char fmt_name(Format fmt);

    void set_reg(u32 reg, u64 value) {
        m_fprs[reg] = std::bit_cast<f64>(value);
    }

private:
    friend class VR4300;
    VR4300& m_vr4300;

    std::array<u64, 32> m_fgrs {};
    std::array<f64, 32> m_fprs {};

    union {
        u32 raw {};
        struct {
            u32 revision : 8;
            u32 implementation : 8;
            u32 : 16;
        } flags;
    } fcr0;

    union {
        u32 raw {};
        struct {
            u32 rounding_mode : 2;
            u32 flags : 5;
            u32 enables : 5;
            u32 cause : 6;
            u32 : 5;
            bool condition : 1;
            bool fs : 1;
            u32 : 7;
        } flags;
    } fcr31;

    static ALWAYS_INLINE u8 get_fmt(const u32 instruction) {
        return Common::bit_range<25, 21>(instruction);
    }

    static ALWAYS_INLINE u8 get_ft(const u32 instruction) {
        return Common::bit_range<20, 16>(instruction);
    }

    static ALWAYS_INLINE u8 get_fs(const u32 instruction) {
        return Common::bit_range<15, 11>(instruction);
    }

    static ALWAYS_INLINE u8 get_fd(const u32 instruction) {
        return Common::bit_range<10, 6>(instruction);
    }

    static ALWAYS_INLINE u8 get_function(const u32 instruction) {
        return Common::bit_range<5, 0>(instruction);
    }
};
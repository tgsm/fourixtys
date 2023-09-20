#include "common/logging.h"
#include "cop1.h"
#include "vr4300.h"

std::string COP1::reg_name(const u32 reg) {
    if (reg <= 31) {
        return fmt::format("f{}", reg);
    }

    UNIMPLEMENTED_MSG("unimplemented name for COP1 reg {}", reg);
}

std::string_view COP1::condition_name(COP1::Conditions condition) {
    using namespace std::string_view_literals;
    static constexpr std::array<std::string_view, 16> condition_names = {
        "F"sv, "UN"sv, "EQ"sv, "UEQ"sv, "OLT"sv, "ULT"sv, "OLE"sv, "ULE"sv, "SF"sv, "NGLE"sv, "SEQ"sv, "NGL"sv, "LT"sv, "NGE"sv, "LE"sv, "NGT"sv,
    };
    return condition_names.at(static_cast<std::size_t>(condition));
}

char COP1::fmt_name(const Format fmt) {
    switch (fmt) {
        case Format::SingleFloating:
            return 'S';
        case Format::DoubleFloating:
            return 'D';
        case Format::Word:
            return 'W';
        case Format::Long:
            return 'L';
        default:
            return '?';
    }
}

void COP1::cvt_d(const u32 instruction) {
    const auto fmt = get_fmt(instruction);
    const auto fs = get_fs(instruction);
    const auto fd = get_fd(instruction);
    LTRACE_FPU("cvt.d.{} ${}, ${}", fmt_name(fmt), reg_name(fd), reg_name(fs));

    switch (fmt) {
        case Format::Word: {
            const u32 word = static_cast<u32>(std::bit_cast<u64>(m_fprs[fs]));
            m_fprs[fd] = static_cast<f64>(word);
            break;
        }

        default:
            UNIMPLEMENTED_MSG("cvt.d.{}", fmt_name(fmt));
    }
}

void COP1::cvt_s(const u32 instruction) {
    const auto fmt = get_fmt(instruction);
    const auto fs = get_fs(instruction);
    const auto fd = get_fd(instruction);
    LTRACE_FPU("cvt.s.{} ${}, ${}", fmt_name(fmt), reg_name(fd), reg_name(fs));

    switch (fmt) {
        case Format::DoubleFloating: {
            const f32 double_to_float = static_cast<f32>(m_fprs[fs]);
            const u64 float_bits = std::bit_cast<u32>(double_to_float);
            m_fprs[fd] = std::bit_cast<f64>(float_bits);
            break;
        }
        case Format::Word: {
            const u32 original_word = static_cast<u32>(std::bit_cast<u64>(m_fprs[fs]));
            const f32 word_to_float = static_cast<f32>(original_word);
            const u64 float_bits = std::bit_cast<u32>(word_to_float);
            m_fprs[fd] = std::bit_cast<f64>(float_bits);
            break;
        }

        default:
            UNIMPLEMENTED_MSG("cvt.s.{}", fmt_name(fmt));
    }
}

void COP1::div(const u32 instruction) {
    const auto fmt = get_fmt(instruction);
    const auto ft = get_ft(instruction);
    const auto fs = get_fs(instruction);
    const auto fd = get_fd(instruction);
    LTRACE_FPU("div.{} ${}, ${}, ${}", fmt_name(fmt), reg_name(fd), reg_name(fs), reg_name(ft));

    if (fmt == Format::SingleFloating) {
        const f32 result = Common::bit_cast_f64_to_f32(m_fprs[fs]) / Common::bit_cast_f64_to_f32(m_fprs[ft]);
        m_fprs[fd] = Common::bit_cast_f32_to_f64(result);
    } else if (fmt == Format::DoubleFloating) {
        m_fprs[fd] = m_fprs[fs] / m_fprs[ft];
    } else {
        UNREACHABLE();
    }
}

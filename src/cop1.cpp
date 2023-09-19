#include "common/logging.h"
#include "cop1.h"

std::string_view COP1::reg_name(const u32 reg) {
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

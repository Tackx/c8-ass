#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <string_view>

enum class Regs
{
    NONE,
    X,
    XY
};

enum class ValueType
{
    NONE,
    N,
    NN,
    NNN
};

enum class TokenType
{
    LABEL,
    MNEM,
    ARG1,
    ARG2
    // Comment?
    // ?
};

struct INSTR
{
    std::string_view mnem;
    std::uint16_t hex;
    Regs regs;
    ValueType val_type;
};

constexpr std::array mnems = {
    INSTR{.mnem = "CLS", .hex = 0x00E0, .regs = Regs::NONE, .val_type = ValueType::NONE},
};

int main()
{
    std::ifstream f{"./test.asm"};

    if (!f.is_open())
    {
        std::println("Cannot open file");

        return 1;
    }

    std::string line;

    for (size_t lineNr = 1; std::getline(f, line); ++lineNr)
    {
        size_t i = 0;

        // Skip lines which are empty or purely comments
        if (line.empty())
        {
            continue;
        }

        if (line.back() == '\r')
        {
            line.pop_back();
        }

        while (i < line.length() && (line[i] == ' ' || line[i] == '\t'))
        {
            i++;
        }

        if (i == line.length() || line[i] == ';')
        {
            continue;
        }

        size_t start = i;

        while (i < line.length() && line[i] != ' ' && line[i] != '\t')
        {
            i++;
        }

        auto mnemonic = std::string_view{line}.substr(start, i - start);

        auto match = std::ranges::find(mnems.begin(), mnems.end(), mnemonic, &INSTR::mnem);

        if (match == mnems.end())
        {
            std::println("Unknown mnemonic: {} on line {}", mnemonic, lineNr);

            return 1;
        }

        std::println("Found matching mnemonic: {}", mnemonic);
    }

    return 0;
}

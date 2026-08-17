#include <fstream>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <array>
#include <print>
#include <string>

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

        while (line[i] == ' ' || line[i] == '\t')
        {
            i++;
        }

        // Skip lines which are just newlines, empty or purely comments
        if (line[i] == '\n' || line[i] == '\0' || line[i] == ';')
        {
            continue;
        }

        std::string mnemonic;

        while (line[i] != ' ' && line[i] != '\0' && line[i] != '\t' && line[i] != '\n')
        {
            mnemonic += line[i];
            i++;
        }

        bool match = false;

        for (size_t k = 0; k < sizeof mnems / sizeof mnems[0]; k++)
        {
            if (mnems[k].mnem == mnemonic)
            {
                std::print("Found matching mnemonic: {}\n", mnemonic);
                match = true;
                break;
            }
        }

        if (!match)
        {
            std::print("Unknown mnemonic: {} on line {}\n", mnemonic, lineNr);

            // TODO: Decide whether to break or continue here
            // TODO2: Return line number
        }
    }

    return 0;
}

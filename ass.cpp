#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

enum class OperandKind
{
    NONE,
    REGISTER,
    VALUE_N,
    VALUE_NN,
    VALUE_NNN
};

struct INSTR
{
    std::string_view mnem;
    std::uint16_t hex;
    std::uint8_t operandCount = 0;
    std::array<OperandKind, 3> operands = {OperandKind::NONE};
};

constexpr std::array mnems = {
    INSTR{.mnem = "CLS", .hex = 0x00E0},
    INSTR{.mnem = "DRW", .hex = 0xD000, .operandCount = 3, .operands = {OperandKind::REGISTER, OperandKind::REGISTER, OperandKind::VALUE_N}},
};

// TODO: Pass the line number for error messages
std::uint16_t encode(const INSTR& instr, const std::vector<std::string_view>& rawArgs)
{

    auto rawHex{instr.hex};

    if (!instr.operands.empty())
    {

        for (size_t i = 0; i < instr.operandCount; i++)
        {

            switch (instr.operands[i])
            {
            case OperandKind::NONE:
                break;

            case OperandKind::REGISTER:

            {
                if (!rawArgs[i].starts_with("V") || rawArgs[i].length() != 2)
                {
                    std::println("Invalid register. Expected register name to start with 'V' and be in the Vx format.");

                    break;
                }

                uint8_t regNumber;
                auto err = std::from_chars(&rawArgs[i][1], &rawArgs[i][1] + 1, regNumber, 16);

                if (err.ec != std::errc{})
                {
                    // TODO: Handle error
                }

                if (regNumber > 0xF)
                {
                    std::println("Invalid register number. Register number must be between 0 and F");

                    break;
                }

                rawHex |= regNumber << (8 - (4 * i));

                break;
            }

            case OperandKind::VALUE_N:
            {
                uint8_t value;
                auto err = std::from_chars(&rawArgs[i][0], &rawArgs[i][0] + 1, value, 10);

                if (err.ec != std::errc{})
                {
                    // TODO: Handle error
                }

                rawHex |= value;

                break;
            }

            case OperandKind::VALUE_NN:
                break;

            case OperandKind::VALUE_NNN:
                break;

            default:
                break;
            }
        }

        for (auto& arg : rawArgs)
        {
            std::println("Arg: {}", arg);
        }
    }

    return rawHex;
}

int assemble()
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

        if (line.empty())
        {
            continue;
        }

        if (line.back() == '\r')
        {
            line.pop_back();
        }

        while (i < line.length() && (line[i] == ' ' || line[i] == '\t' || line[i] == ','))
        {
            i++;
        }

        if (i == line.length() || line[i] == ';')
        {
            continue;
        }

        size_t start = i;

        while (i < line.length() && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',')
        {
            i++;
        }

        auto mnemonic = std::string_view{line}.substr(start, i - start);

        auto match = std::ranges::find(mnems, mnemonic, &INSTR::mnem);

        if (match == mnems.end())
        {
            std::println("Unknown mnemonic: {} on line {}", mnemonic, lineNr);

            return 1;
        }

        std::println("Found matching mnemonic: {}", mnemonic);

        std::vector<std::string_view> rawOps{};

        while (i < line.length())
        {
            // We may have ended on a space/tab/comma above
            while (i < line.length() && (line[i] == ' ' || line[i] == '\t' || line[i] == ','))
            {
                i++;
            }

            if (i >= line.length())
            {
                std::println("End of line reached while parsing ops");

                break;
            }

            if (line[i] == ';')
            {
                std::println("Found comment, stopping parsing line {}", lineNr);

                break;
            }

            auto opStart = i;
            while (i < line.length() && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',')
            {
                i++;
            }

            auto op = std::string_view{line}.substr(opStart, i - opStart);

            rawOps.push_back(op);
        }

        auto hex = encode(*match, rawOps);

        std::println("Final hex: {:x}", hex);

        // TODO: Write the encoded instruction into a binary output, finally
    }

    return 0;
}

int main()
{
    // TODO: Pass args to be able to read files from there
    // TODO2: Also read files from redirected stdin
    return assemble();
}

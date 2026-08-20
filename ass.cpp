#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

enum class OperandKind
{
    NONE,
    REGISTER,
    VALUE_N,
    VALUE_NN,
    VALUE_NNN
};

struct Instruction
{
    std::string_view mnem;
    std::uint16_t hex;
    std::uint8_t operandCount = 0;
    std::array<OperandKind, 3> operands = {OperandKind::NONE};
};

constexpr std::array mnems = {
    Instruction{.mnem = "CLS", .hex = 0x00E0},
    Instruction{.mnem = "DRW", .hex = 0xD000, .operandCount = 3, .operands = {OperandKind::REGISTER, OperandKind::REGISTER, OperandKind::VALUE_N}},
};

std::unordered_map<std::string, uint16_t> labelMemoryMap;

// TODO: Pass the line number for error messages
std::uint16_t encode(const Instruction& instr, const std::vector<std::string_view>& rawArgs)
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

// TODO: Rework, this is ugly but cba rn
int firstPass(char** args)
{
    std::ifstream fi{args[1]};
    if (!fi.is_open())
    {
        std::println("Cannot open input file");

        return 1;
    }

    std::ofstream fo{args[2], std::ios_base::binary};
    ;
    if (!fo.is_open())
    {
        std::println("Cannot open output file");

        return 1;
    }

    std::string line;

    for (size_t lineNr = 1; std::getline(fi, line); ++lineNr)
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

        auto token = std::string_view{line}.substr(start, i - start);

        if (line[i - 1] == ':')
        {
            std::println("IT'S A LABEL");

            labelMemoryMap[std::string{token}] = 0x200 + (lineNr - 1);
        }
    }

    return 0;
};

int secondPass(char** args)
{
    std::ifstream fi{args[1]};
    if (!fi.is_open())
    {
        std::println("Cannot open input file");

        return 1;
    }

    std::ofstream fo{args[2], std::ios_base::binary};
    ;
    if (!fo.is_open())
    {
        std::println("Cannot open output file");

        return 1;
    }

    std::string line;

    for (size_t lineNr = 1; std::getline(fi, line); ++lineNr)
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

        if (line[i - 1] == ':')
        {
            i++; // TODO: ADD A BOUNDS CHECK (there could be nothing after the label..)
            start = i;
            // Labels were parsed in the first pass
            while (i < line.length() && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',')
            {
                i++;
            }
        }

        auto token = std::string_view{line}.substr(start, i - start);

        auto match = std::ranges::find(mnems, token, &Instruction::mnem);

        if (match == mnems.end())
        {
            std::println("Unknown mnemonic: {} on line {}", token, lineNr);

            return 1;
        }

        std::println("Found matching mnemonic: {}", token);

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

        fo.put(static_cast<char>(hex >> 8));
        fo.put(static_cast<char>(hex & 0xFF));
    }

    return 0;
}

int assemble(char** args)
{
    int r = 0;

    r = firstPass(args);
    if (r != 0)
    {
        return r;
    }

    r = secondPass(args);

    return r;
}

constexpr std::string_view USAGE_MSG = "Usage: ass INPUT OUTPUT";

int main(int argc, char** argv)
{

    // TODO: Enable reading from a redirected stream
    if (argc <= 2)
    {

        if (argc <= 1)
        {
            std::println("Missing input path.");
        }

        std::println("Missing output path.");

        std::println("{}", USAGE_MSG);

        return 1;
    }

    return assemble(argv);
}

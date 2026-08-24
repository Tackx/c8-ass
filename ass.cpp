#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "ass.h"

// TODO: Pass the line number for error messages
std::uint16_t encode(const Instruction& instr, const std::vector<std::string_view>& rawArgs)
{

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

    uint16_t memPointer{0x200};

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

        auto token = std::string{line}.substr(start, i - start);

        if (line[i - 1] == ':')
        {
            if (labelMemoryMap.contains(token))
            {
                std::println("Found duplicate label on line {}. Label {} is already defined on line {}.", lineNr,
                             std::string_view{token}.substr(0, token.length() - 1), labelMemoryMap[token].line);

                return 1;
            }

            labelMemoryMap[token] = Label{.addr = memPointer, .line = lineNr};

            std::println("IT'S A LABEL: {}", labelMemoryMap.at(token).addr);
        }

        memPointer += 2;
    }

    return 0;
};

Instruction parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs, size_t lineNr)
{

    if (rawArgs.size() > 3)
    {

        // TODO: Refactor this..
        throw "Too many arguments";
    }

    std::array<OperandKind, 3> ops{};

    for (size_t i = 0; i < ops.size(); i++)
    {

        auto op = rawArgs[i];

        if (op == "[I]" || op == "[i]")
        {
            ops[i] = OperandKind::I_MEM;

            continue;
        }

        if (op == "I" || op == "i")
        {
            ops[i] = OperandKind::I_REG;

            continue;
        }

        if (op == "V0" || op == "v0")
        {
            ops[i] = OperandKind::V0;

            continue;
        }

        if (op.starts_with("V") || op.starts_with("v"))
        {
            ops[i] = OperandKind::REGISTER;

            continue;
        }

        if (op == "DT" || op == "dt")
        {
            ops[i] = OperandKind::DT;

            continue;
        }

        if (op == "K" || op == "k")
        {
            ops[i] = OperandKind::KEY;

            continue;
        }

        if (op == "ST" || op == "st")
        {
            ops[i] = OperandKind::ST;

            continue;
        }

        if (op == "LF" || op == "lf" || op == "F" || op == "f")
        {
            ops[i] = OperandKind::FONT;

            continue;
        }

        if (op == "B" || op == "b")
        {
            ops[i] = OperandKind::BCD;

            continue;
        }

        ops[i] = OperandKind::LITERAL;
    }

    // auto match = std::ranges::find(opTable, ops, &InstructionDefinition::operands);

    auto match = std::ranges::find_if(opTable, [&](InstructionDefinition instr) { return instr.mnem == mnem && instr.operands == ops; });

    if (match == opTable.end())
    {
        throw "Failed to find mnemonic with matching operand kinds";
    }

    InstructionDefinition instr = *match;

    std::println("Found matching instruction in op table");

    std::array<uint16_t, 3> parsedOpValues{};

    auto rawHex{instr.hex};
    auto sourceValueBase = 10;

    // TODO: Iterate over rawArgs. Parse each arg based on the found instr's operands array and push the parsed value into parsedOpValues. Then construct the
    // Instruction struct.
    if (!instr.operands.empty())
    {

        for (size_t i = 0; i < instr.operandCount; i++)
        {
            std::string str{rawArgs[i]};
            if (str.starts_with("0x"))
            {
                sourceValueBase = 16;
                str = str.substr(2);
            }

            switch (instr.operands[i])
            {
            case OperandKind::NONE:
                break;

            case OperandKind::REGISTER:
            {
                if (!str.starts_with("V") || str.length() != 2)
                {
                    std::println("Invalid register. Expected register name to start with 'V' and be in the Vx format.");

                    break;
                }

                uint8_t regNumber;
                auto err = std::from_chars(&str[1], &str[1] + 1, regNumber, sourceValueBase);

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
                auto err = std::from_chars(&str[0], &str[0] + 2, value, sourceValueBase);

                if (err.ec != std::errc{})
                {
                    // TODO: Handle error
                }

                // TODO: More validations.. (e.g. reject values > 15)

                rawHex |= value;

                break;
            }

            case OperandKind::VALUE_NN:
            {
                uint8_t value;
                auto err = std::from_chars(&str[0], &str[0] + 3, value, sourceValueBase);

                if (err.ec != std::errc{})
                {
                    // TODO: Handle error
                }

                // TODO: More validations.. (e.g. reject values > 255)

                rawHex |= value;

                break;
            }

            case OperandKind::ADDRESS:
            {
                uint8_t value;
                auto err = std::from_chars(&str[0], &str[0] + 5, value, sourceValueBase);

                if (err.ec != std::errc{})
                {
                    // TODO: Handle error
                }

                if (err.ptr != str.data() + str.size())
                {
                }

                // TODO: More validations.. (e.g. reject values > 4096)

                rawHex |= value;

                break;
            }

            default:
                break;
            }
        }

        for (auto& arg : rawArgs)
        {
            std::println("Arg: {}", arg);
        }
    }

    // for (uint8_t i = 0; i < ops.size(); i++)
    // {
    //     uint8_t base = 10;
    //     auto op = rawArgs[i];

    //     if (op.starts_with("0x"))
    //     {
    //         base = 16;
    //         op.remove_prefix(2);
    //     }

    //     uint16_t parsed;
    //     auto err = std::from_chars(&op[0], &op[op.length() - 1], parsed, base);

    //     if (err.ec != std::errc{})
    //     {
    //         throw "Failed parsing arg"; // TODO
    //     }
    // }

    return Instruction{
        .def = *match,
        .operandValues = rawArgs // TODO: Replace labels with memory addresses..
    };
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
            i++;
            if (line.length() <= i)
            {
                // The label is on a standalone line, no mnem to parse here
                continue;
            }

            start = i;
            // Labels were parsed in the first pass, continue to the mnemonic
            while (i < line.length() && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',')
            {
                i++;
            }
        }

        auto mnem = std::string_view{line}.substr(start, i - start);

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

        auto parsedInstruction = parseInstruction(mnem, rawOps, lineNr);

        auto hex = encode(parsedInstruction, rawOps);

        std::println("Final hex: {:x}", hex);

        fo.put(static_cast<char>(hex >> 8));
        fo.put(static_cast<char>(hex & 0xFF));
    }

    return 0;
}

int assemble(int argc, char** args)
{
    static constexpr std::string_view USAGE_MSG = "Usage: ass INPUT OUTPUT";

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

    int r = 0;

    r = firstPass(args);
    if (r != 0)
    {
        return r;
    }

    r = secondPass(args);

    return r;
}

int main(int argc, char** argv)
{

    return assemble(argc, argv);
}

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
    ADDRESS, // AKA NNN
    I_REG,
    V0,
    DT,
    KEY,
    ST,
    FONT,
    BCD,
    I_MEM
};

struct InstructionDefinition
{
    std::string_view mnem;
    std::uint16_t hex;
    std::uint8_t operandCount = 0;
    std::array<OperandKind, 3> operands;
};

struct Instruction
{
    const InstructionDefinition* def;
    std::array<uint16_t, 3> operandValues;
    bool label; // Should be set to true operandValues contain a label which needs to be resolved
};

constexpr std::array ops = {
    InstructionDefinition{.mnem = "CLS", .hex = 0x00E0},
    InstructionDefinition{.mnem = "RET", .hex = 0x00EE},
    InstructionDefinition{.mnem = "JP", .hex = 0x1000, .operandCount = 1, .operands = {OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "CALL", .hex = 0x2000, .operandCount = 1, .operands = {OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "SE", .hex = 0x3000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "SNE", .hex = 0x4000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "SE", .hex = 0x5000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0x6000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "ADD", .hex = 0x7000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "LD", .hex = 0x8000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "OR", .hex = 0x8001, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "AND", .hex = 0x8002, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "XOR", .hex = 0x8003, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "ADD", .hex = 0x8004, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SUB", .hex = 0x8005, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SHR", .hex = 0x8006, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SUBN", .hex = 0x8007, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SHL", .hex = 0x800E, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SNE", .hex = 0x9000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xA000, .operandCount = 2, .operands = {OperandKind::I_REG, OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "JP", .hex = 0xB000, .operandCount = 2, .operands = {OperandKind::V0, OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "RND", .hex = 0xC000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "DRW", .hex = 0xD000, .operandCount = 3, .operands = {OperandKind::REGISTER, OperandKind::REGISTER, OperandKind::VALUE_N}},
    InstructionDefinition{.mnem = "SKP", .hex = 0xE09E, .operandCount = 1, .operands = {OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SKNP", .hex = 0xE0A1, .operandCount = 1, .operands = {OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF007, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::DT}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF00A, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::KEY}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF015, .operandCount = 2, .operands = {OperandKind::DT, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF018, .operandCount = 2, .operands = {OperandKind::ST, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "ADD", .hex = 0xF01E, .operandCount = 2, .operands = {OperandKind::I_REG, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF029, .operandCount = 2, .operands = {OperandKind::FONT, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF033, .operandCount = 2, .operands = {OperandKind::BCD, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF055, .operandCount = 2, .operands = {OperandKind::I_MEM, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF065, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::I_MEM}},
};

struct Label
{
    uint16_t addr;
    size_t line;
};

std::unordered_map<std::string, Label> labelMemoryMap;

// TODO: Pass the line number for error messages
std::uint16_t encode(const InstructionDefinition& instr, const std::vector<std::string_view>& rawArgs)
{

    auto rawHex{instr.hex};
    auto sourceValueBase = 10;

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
                std::println("Found duplicate label on line {}. Label is {} already defined on line {}.", lineNr,
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

        auto token = std::string_view{line}.substr(start, i - start);

        auto match = std::ranges::find(ops, token, &InstructionDefinition::mnem);

        if (match == ops.end())
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

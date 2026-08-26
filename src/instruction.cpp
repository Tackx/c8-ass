#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <print>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "instruction.h"
#include "label.h"

namespace ass
{

bool isCompatible(Operand first, Operand second)
{
    if (first.literalType.has_value() && second.literalType.has_value())
    {
        return first.argType == second.argType && first.literalType == second.literalType;
    }

    return first.argType == second.argType;
}

Instruction parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs, size_t lineNr)
{

    if (rawArgs.size() > 3)
    {

        // TODO: Refactor this..
        throw "Too many arguments";
    }

    // Parse operand type for each arg
    // Needed so we know how to parse the values down the line
    std::array<Operand, 3> parsedOperands{};

    if (rawArgs.size() > 0)
    {
        for (size_t i = 0; i < rawArgs.size(); i++)
        {

            auto op = rawArgs[i];

            if (op == "[I]" || op == "[i]")
            {
                parsedOperands[i] = {ArgType::I_MEM};

                continue;
            }

            if (op == "I" || op == "i")
            {
                parsedOperands[i] = {ArgType::I_REG};

                continue;
            }

            // if (op == "V0" || op == "v0")
            // {
            //     parsedOperands[i] = {ArgType::V0};

            //     continue;
            // }

            if (op.starts_with("V") || op.starts_with("v"))
            {
                parsedOperands[i] = {ArgType::REGISTER};

                continue;
            }

            if (op == "DT" || op == "dt")
            {
                parsedOperands[i] = {ArgType::DT};

                continue;
            }

            if (op == "K" || op == "k")
            {
                parsedOperands[i] = {ArgType::KEY};

                continue;
            }

            if (op == "ST" || op == "st")
            {
                parsedOperands[i] = {ArgType::ST};

                continue;
            }

            if (op == "LF" || op == "lf" || op == "F" || op == "f")
            {
                parsedOperands[i] = {ArgType::FONT};

                continue;
            }

            if (op == "B" || op == "b")
            {
                parsedOperands[i] = {ArgType::BCD};

                continue;
            }

            parsedOperands[i] = {ArgType::LITERAL};
        }
    }

    // Find matching instruction/op
    auto match = std::ranges::find_if(opTable,
                                      [&](const InstructionDefinition& instr)
                                      {
                                          if (mnem != instr.mnem)
                                          {
                                              return false;
                                          }

                                          if (rawArgs.size() != instr.operandCount)
                                          {
                                              return false;
                                          }

                                          bool compatible = true;
                                          for (size_t i = 0; i < instr.operandCount; i++)
                                          {
                                              if (!isCompatible(parsedOperands[i], instr.operands[i]))
                                              {
                                                  compatible = false;

                                                  break;
                                              }
                                          }

                                          return compatible;
                                      });

    if (match == opTable.end())
    {
        throw "Failed to find mnemonic with matching operand kinds";
    }

    InstructionDefinition instr = *match;

    std::println("Found matching instruction in op table");

    // Parse the arg values
    std::array<uint16_t, 3> parsedOpValues{};

    // Iterate over rawArgs. Parse each arg based on the found instr's operands array and push the parsed value into parsedOpValues. Then construct the
    // Instruction struct.
    auto rawHex{instr.hex};
    if (!instr.operands.empty())
    {
        for (size_t i = 0; i < instr.operandCount; i++)
        {
            auto sourceValueBase = 10;

            std::string str{rawArgs[i]};
            if (str.starts_with("0x"))
            {
                sourceValueBase = 16;
                str = str.substr(2);
            }

            switch (instr.operands[i].argType)
            {
            case ArgType::NONE:
                break;

            case ArgType::REGISTER:
            {
                sourceValueBase = 16;

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

                parsedOpValues[i] = regNumber;
                rawHex |= regNumber << (8 - (4 * i));

                break;
            }

            case ArgType::LITERAL:
            {

                auto literalType = instr.operands[i].literalType;

                if (!literalType.has_value())
                {
                    throw "Cannot parse literal (arg specified as literal, but found no value for it)";
                }

                if (literalType == LiteralType::VALUE_N)
                {
                    uint8_t value;
                    auto err = std::from_chars(&str[0], &str[0] + 2, value, sourceValueBase);

                    if (err.ec != std::errc{})
                    {
                        // TODO: Handle error
                    }

                    // TODO: More validations.. (e.g. reject values > 15)

                    parsedOpValues[i] = value;
                    rawHex |= value;

                    break;
                };

                if (literalType == LiteralType::VALUE_NN)
                {
                    uint8_t value;
                    auto err = std::from_chars(&str[0], &str[0] + 3, value, sourceValueBase);

                    if (err.ec != std::errc{})
                    {
                        // TODO: Handle error
                    }

                    // TODO: More validations.. (e.g. reject values > 255)

                    parsedOpValues[i] = value;
                    rawHex |= value;

                    break;
                };

                if (literalType == LiteralType::ADDRESS)
                {
                    // Labels can only be used as address placeholders
                    // Check if label map contains the literal and if so,
                    // turn the label into the assigned address
                    if (labelMemoryMap.contains(str))
                    {
                        parsedOpValues[i] = labelMemoryMap[str].addr;
                        rawHex |= labelMemoryMap[str].addr;

                        break;
                    }

                    uint16_t value;
                    auto err = std::from_chars(&str[0], &str[0] + 5, value, sourceValueBase);

                    if (err.ec != std::errc{})
                    {
                        // TODO: Handle error
                    }

                    if (err.ptr != str.data() + str.size())
                    {
                    }

                    // TODO: More validations.. (e.g. reject values > 4096)

                    parsedOpValues[i] = value;
                    rawHex |= value;

                    break;
                };
            }

            default:
                break;
            }
        }
    }

    return Instruction{
        .def = *match,
        .operandValues = parsedOpValues, // TODO: Replace labels with memory addresses..
        .encodedHex = rawHex,
    };
};
} // namespace ass
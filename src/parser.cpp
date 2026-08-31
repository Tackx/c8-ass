#include "parser.h"
#include "instruction.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "label.h"

namespace ass
{
Parser::Parser(const std::string& inPath) : m_lineNr{0}, m_fi{inPath}
{
    if (!m_fi.is_open())
    {
        throw std::runtime_error("Cannot open input file");
    }
}

std::optional<Instruction> Parser::parseLine()
{

    while (std::getline(m_fi, m_line))
    {
        ++m_lineNr;

        size_t i = 0;

        if (m_line.empty())
        {
            continue;
        }

        if (m_line.back() == '\r')
        {
            m_line.pop_back();
        }

        while (i < m_line.length() && (Parser::isWhitespace(m_line[i]) || Parser::isArgSeparator(m_line[i])))
        {
            i++;
        }

        if (i == m_line.length() || Parser::isComment(m_line[i]))
        {
            continue;
        }

        size_t start = i;

        while (i < m_line.length() && !Parser::isWhitespace(m_line[i]) && !Parser::isComment(m_line[i]) && !Parser::isArgSeparator(m_line[i]))
        {
            i++;
        }

        if (m_line[i - 1] == ':')
        {
            i++;
            if (m_line.length() <= i || Parser::isArgSeparator(m_line[i]))
            {
                // The label is on a standalone line, no mnem to parse here
                continue;
            }

            start = i;
            // Labels were parsed in the first pass, continue to the mnemonic
            while (i < m_line.length() && !Parser::isWhitespace(m_line[i]) && !Parser::isComment(m_line[i]) && !Parser::isArgSeparator(m_line[i]))
            {
                i++;
            }
        }

        auto mnem = std::string_view{m_line}.substr(start, i - start);

        std::vector<std::string_view> rawOps{};

        while (i < m_line.length())
        {
            // We may have ended on a space/tab/comma above
            while (i < m_line.length() && (Parser::isWhitespace(m_line[i]) || Parser::isArgSeparator(m_line[i])))
            {
                i++;
            }

            if (i >= m_line.length())
            {
                std::println("End of line reached while parsing ops");

                break;
            }

            if (Parser::isComment(m_line[i]))
            {
                std::println("Found comment, stopping parsing line {}", m_lineNr);

                break;
            }

            auto opStart = i;
            while (i < m_line.length() && !Parser::isWhitespace(m_line[i]) && !Parser::isComment(m_line[i]) && !Parser::isArgSeparator(m_line[i]))
            {
                i++;
            }

            auto op = std::string_view{m_line}.substr(opStart, i - opStart);

            rawOps.push_back(op);
        }

        auto parsedInstruction = this->parseInstruction(mnem, rawOps);

        std::println("Final hex: {:x}", parsedInstruction.encodedHex);

        return parsedInstruction;
    }

    return {};
}

bool Parser::isWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

bool Parser::isComment(char c)
{
    return c == ';';
}

bool Parser::isArgSeparator(char c)
{
    return c == ',';
}

Instruction Parser::parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs)
{
    if (rawArgs.size() > 3)
    {
        throw std::runtime_error("Too many arguments");
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
        throw std::runtime_error(std::format("Failed to find mnemonic with matching operand kinds.\nLine: {}\nMnemonic: {}", m_lineNr, mnem));
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
                    throw std::runtime_error("Cannot parse literal (arg specified as literal, but found no value for it)");
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

                    // TODO: Add check if the value is numeric. If not, it's an invalid label.

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
        .operandValues = parsedOpValues,
        .encodedHex = rawHex,
    };
}
} // namespace ass
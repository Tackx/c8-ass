#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

#include "instruction.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"

namespace ass
{

Parser::Parser(std::string_view lines) : m_lines{lines}, m_lineNr{0}
{
}

// TODO: Rewrite to accept std::vector<Token>
void Parser::parseLabels(std::string_view textContent)
{
    uint16_t memPointer{0x200};

    size_t lineNr = 0;
    for (auto str : textContent | std::views::split('\n'))
    {
        lineNr++;

        std::string_view line{str};

        size_t i = 0;

        if (line.empty())
        {
            continue;
        }

        if (line.back() == '\r')
        {
            line.remove_suffix(1);
        }

        while (i < line.length() && (Lexer::isWhitespace(line[i]) || Lexer::isArgSeparator(line[i])))
        {
            i++;
        }

        if (i == line.length() || Lexer::isComment(line[i]))
        {
            continue;
        }

        size_t start = i;

        // TODO: Add another static helper isLabelEnd
        while (i < line.length() && !Lexer::isWhitespace(line[i]) && !Lexer::isComment(line[i]) && !Lexer::isArgSeparator(line[i]) && line[i] != ':')
        {
            i++;
        }

        std::string token{line.substr(start, i - start)};

        if (i < line.length() && line[i] == ':')
        {
            if (m_labelMemoryMap.contains(token))
            {
                throw std::runtime_error(std::format("Found duplicate label on line {}. Label {} is already defined on line {}.", lineNr,
                                                     std::string_view{token}.substr(0, token.length() - 1), m_labelMemoryMap[token].line));
            }

            m_labelMemoryMap[token] = Label{.addr = memPointer, .line = lineNr};

            std::println("IT'S A LABEL: {}", m_labelMemoryMap.at(token).addr);

            if (i < line.length())
            {
                i++; // Move cursor forward by one char, as we ended on ':'
            }

            while (i < line.length() && (Lexer::isWhitespace(line[i])))
            {
                i++;
            }

            // Continue to avoid incrementing the memory pointer, but only if it's a standalone label (on its own line)
            if (i == line.length() || Lexer::isComment(line[i]))
            {
                continue;
            }

            // TODO: It would make sense to do semantic checks here in the first pass too to have feedback and stop the process earlier
        }

        memPointer += 2;
    }
}

std::vector<Instruction> Parser::parseInstructions(const std::vector<Token>& tokens)
{
    std::vector<Instruction> out;

    std::string_view mnem{};
    std::vector<std::string_view> rawArgs{};

    // Is this the first identifier of a line?
    // If so, it's either a label or a mnemonic
    bool firstIdentifier = true;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const auto& token = tokens[i];

        if (i + 1 < tokens.size() && token.type == TokenType::Identifier && tokens[i + 1].type == TokenType::Colon)
        {
            // It's a label, skip both tokens (labels are processed in the first pass)
            i++;
        }
        else if (token.type == TokenType::Identifier && firstIdentifier)
        {
            // It's a mnemonic
            mnem = token.text;
        }
        else if (token.type == TokenType::Identifier || token.type == TokenType::Number)
        {
            // It's an arg
            rawArgs.push_back(token.text);
        }
        else if (token.type == TokenType::Newline && mnem != "")
        {
            auto instr = parseInstruction(mnem, rawArgs);
            out.push_back(instr);

            mnem = "";
            rawArgs = {};
            firstIdentifier = true;

            continue;
        }
        else if (token.type == TokenType::Newline)
        {
            firstIdentifier = true;

            continue;
        }

        firstIdentifier = false;

        // TODO: Do we care about the end token?
    }

    return out;
}

// TODO: Rewrite to take a vector of Token structs instead of the raw strings
Instruction Parser::parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs)
{
    if (rawArgs.size() > 3)
    {
        throw std::runtime_error("Too many arguments");
    }

    // Parse operand type for each arg
    // Needed so we know how to parse the values down the line
    std::array<Operand, 3> parsedOperandTypes{};

    if (rawArgs.size() > 0)
    {
        for (size_t i = 0; i < rawArgs.size(); i++)
        {

            auto op = rawArgs[i];

            if (op == "[I]" || op == "[i]")
            {
                parsedOperandTypes[i] = {ArgType::I_MEM};

                continue;
            }

            if (op == "I" || op == "i")
            {
                parsedOperandTypes[i] = {ArgType::I_REG};

                continue;
            }

            // if (op == "V0" || op == "v0")
            // {
            //     parsedOperands[i] = {ArgType::V0};

            //     continue;
            // }

            if (op.starts_with("V") || op.starts_with("v"))
            {
                parsedOperandTypes[i] = {ArgType::REGISTER};

                continue;
            }

            if (op == "DT" || op == "dt")
            {
                parsedOperandTypes[i] = {ArgType::DT};

                continue;
            }

            if (op == "K" || op == "k")
            {
                parsedOperandTypes[i] = {ArgType::KEY};

                continue;
            }

            if (op == "ST" || op == "st")
            {
                parsedOperandTypes[i] = {ArgType::ST};

                continue;
            }

            if (op == "LF" || op == "lf" || op == "F" || op == "f")
            {
                parsedOperandTypes[i] = {ArgType::FONT};

                continue;
            }

            if (op == "B" || op == "b")
            {
                parsedOperandTypes[i] = {ArgType::BCD};

                continue;
            }

            parsedOperandTypes[i] = {ArgType::LITERAL};
        }
    }

    // Find matching instruction/op
    auto match = std::ranges::find_if(opTable,
                                      [&](const InstructionDefinition& instr)
                                      {
                                          if (!equalsIgnoreCase(mnem, instr.mnem))
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
                                              if (!isCompatible(parsedOperandTypes[i], instr.operands[i]))
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
            // uint8_t

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

                if ((!str.starts_with("V") && !str.starts_with("v")) || str.length() != 2)
                {
                    throw std::runtime_error("Invalid register. Expected register name to start with 'V' and be in the Vx format.");

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
                    throw std::runtime_error("Invalid register number. Register number must be between 0 and F");

                    break;
                }

                parsedOpValues[i] = regNumber;

                // TODO: This is ugly, divide ArgType::REGISTER into REGISTER_X and REGISTER_Y, each having its own case in this switch
                if (i > 0 && instr.operands[i - 1].argType != ArgType::REGISTER)
                {
                    rawHex |= regNumber << 8;
                }
                else
                {
                    rawHex |= regNumber << (8 - (4 * i));
                }

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
                    if (m_labelMemoryMap.contains(str))
                    {
                        parsedOpValues[i] = m_labelMemoryMap[str].addr;
                        rawHex |= m_labelMemoryMap[str].addr;

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
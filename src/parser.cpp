#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <print>
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

Parser::Parser()
{
}

void Parser::parseLabels(const std::vector<Token>& tokens)
{
    uint16_t memPointer{0x200};

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const auto& token = tokens[i];

        if (token.type == TokenType::Identifier && i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Colon)
        {
            // It's a label
            const auto& key = std::string{token.text};

            if (m_labelMemoryMap.contains(key))
            {
                throw std::runtime_error(std::format("Found duplicate label on line {}:{}. Label {} is already defined on line {}.", token.line, token.col,
                                                     token.text.substr(0, token.text.length() - 1), m_labelMemoryMap[key].line));
            }

            m_labelMemoryMap[key] = Label{.addr = memPointer, .line = token.line};

            std::println("IT'S A LABEL: {}", m_labelMemoryMap.at(key).addr);

            i++; // Skip the colon

            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Newline)
            {
                i++; // Skip the newline

                continue;
            }
            else
            {
                memPointer += 2;
            }
        }

        // If the previous token is not a newline and this one is, increment the pointer
        if (i > 0 && tokens[i - 1].type != TokenType::Newline && token.type == TokenType::Newline)
        {
            memPointer += 2;
        }

        // TODO: It would make sense to do semantic checks here in the first pass too to have feedback and stop the process earlier
    }
}

std::vector<Instruction> Parser::parseInstructions(const std::vector<Token>& tokens)
{
    std::vector<Instruction> out;

    std::string_view mnem{};
    std::vector<Token> rawArgs{};

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
            rawArgs.push_back(token);
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

uint8_t Parser::parseRegister(std::string_view registerString)
{
    if ((!registerString.starts_with("V") && !registerString.starts_with("v")) || registerString.length() != 2)
    {
        throw std::runtime_error("Invalid register. Expected register name to start with 'V' and be in the Vx format.");
    }

    uint8_t regNumber;
    auto err = std::from_chars(&registerString[1], &registerString[1] + 1, regNumber, 16);

    if (err.ec != std::errc{} || err.ptr != &registerString[1] + 1)
    {
        throw std::runtime_error(std::format("Failed to parse register number: {}", std::make_error_code(err.ec).message()));
    }

    if (regNumber > 0xF)
    {
        throw std::runtime_error("Invalid register number. Register number must be between 0 and F");
    }

    return regNumber;
}

Instruction Parser::parseInstruction(std::string_view mnem, const std::vector<Token>& args)
{
    if (args.size() > 3)
    {
        throw std::runtime_error("Too many arguments");
    }

    // Parse operand type for each arg
    // Needed so we know how to parse the values down the line
    std::array<Operand, 3> parsedOperandTypes{};

    if (args.size() > 0)
    {
        for (size_t i = 0; i < args.size(); i++)
        {

            auto token = args[i];

            // [i] || [I]
            if (token.type == TokenType::LBracket && i + 2 < args.size() && args[i + 1].type == TokenType::Identifier &&
                (args[i + 1].text == "I" || args[i + 1].text == "i") && args[i + 2].type == TokenType::RBracket)
            {
                parsedOperandTypes[i] = {ArgType::I_MEM};

                i += 2; // Skip the next 2 tokens
            }

            else if (token.type == TokenType::Identifier && (token.text == "I" || token.text == "i"))
            {
                parsedOperandTypes[i] = {ArgType::I_REG};
            }

            else if (token.type == TokenType::Identifier && (token.text.starts_with("V") || token.text.starts_with("v")) && i > 0 &&
                     parsedOperandTypes[i - 1].argType == ArgType::REGISTER_X)
            {
                parsedOperandTypes[i] = {ArgType::REGISTER_Y};
            }

            else if (token.type == TokenType::Identifier && (token.text.starts_with("V") || token.text.starts_with("v")))
            {
                parsedOperandTypes[i] = {ArgType::REGISTER_X};
            }

            else if (token.type == TokenType::Identifier && (token.text == "DT" || token.text == "dt"))
            {
                parsedOperandTypes[i] = {ArgType::DT};
            }

            else if (token.type == TokenType::Identifier && (token.text == "K" || token.text == "k"))
            {
                parsedOperandTypes[i] = {ArgType::KEY};
            }

            else if (token.type == TokenType::Identifier && (token.text == "ST" || token.text == "st"))
            {
                parsedOperandTypes[i] = {ArgType::ST};
            }

            else if (token.type == TokenType::Identifier && (token.text == "LF" || token.text == "lf" || token.text == "F" || token.text == "f"))
            {
                parsedOperandTypes[i] = {ArgType::FONT};
            }

            else if (token.type == TokenType::Identifier && (token.text == "B" || token.text == "b"))
            {
                parsedOperandTypes[i] = {ArgType::BCD};
            }

            else
            {
                parsedOperandTypes[i] = {ArgType::LITERAL};
            }
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

                                          if (args.size() != instr.operandCount)
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
        throw std::runtime_error(std::format("Failed to find mnemonic with matching operand kinds.\nLine: {}\nMnemonic: {}", args[0].line, mnem));
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

            std::string str{args[i].text};
            if (str.starts_with("0x"))
            {
                sourceValueBase = 16;
                str = str.substr(2);
            }

            switch (instr.operands[i].argType)
            {
            case ArgType::NONE:
                break;

            case ArgType::REGISTER_X:
            {
                auto regNumber = parseRegister(str);
                parsedOpValues[i] = regNumber;

                rawHex |= regNumber << 8;

                break;
            }

            case ArgType::REGISTER_Y:
            {
                auto regNumber = parseRegister(str);
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
                    if (m_labelMemoryMap.contains(str))
                    {
                        parsedOpValues[i] = m_labelMemoryMap[str].addr;
                        rawHex |= m_labelMemoryMap[str].addr;

                        break;
                    }

                    // TODO: Add check if the value is numeric. If not and it's not a valid label, it's an invalid address.

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
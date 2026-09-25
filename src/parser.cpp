#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "instruction.h"
#include "lexer.h"
#include "loader.h"
#include "parser.h"
#include "util.h"

namespace ass
{

Parser::Parser()
{
}

bool Parser::isDirective(const Token& mnem)
{
    auto foundDirective =
        std::find_if(supportedDirectives.begin(), supportedDirectives.end(), [&](const auto& elem) { return equalsIgnoreCase(elem, mnem.text); });

    if (foundDirective != supportedDirectives.end())
    {
        return true;
    }

    return false;
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

            const auto& foundKey = m_labelMemoryMap.find(key);
            if (foundKey != m_labelMemoryMap.end())
            {
                throw std::runtime_error(
                    std::format(
                        "{}:{}:{}: Duplicate label. Label {} is already defined on line {}:{}", ass::filename, token.line, token.col, token.text, ass::filename,
                        foundKey->second.line
                    )
                );
            }

            m_labelMemoryMap[key] = Label{.addr = memPointer, .line = token.line};

            i++; // Skip the colon

            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Newline)
            {
                i++; // Skip the newline

                continue;
            }
        }

        else if (token.type == TokenType::Identifier && isDirective(token))
        {
            if (equalsIgnoreCase(token.text, "DB") || equalsIgnoreCase(token.text, ".byte"))
            {
                i++;

                while (i < tokens.size() && (tokens[i].type != TokenType::Newline && tokens[i].type != TokenType::End))
                {
                    if (tokens[i].type == TokenType::Number)
                    {
                        memPointer++;
                    }

                    i++;
                }

                continue;
            }

            if (equalsIgnoreCase(token.text, "INCBIN"))
            {
                if (i + 1 >= tokens.size() || tokens[i + 1].type != TokenType::Identifier)
                {
                    throw std::runtime_error(
                        std::format("{}:{}:{}: Expected an identifier with filename for the INCBIN directive to follow", ass::filename, token.line, token.col)
                    );
                }

                const auto& filepathToken = tokens[i + 1];
                const auto& inputPath = std::string{filepathToken.text};

                auto exists = std::filesystem::exists(inputPath);
                if (!exists)
                {
                    throw std::runtime_error(
                        std::format("{}:{}:{}: The provided filepath does not exist: {}", ass::filename, filepathToken.line, filepathToken.col, inputPath)
                    );
                }

                auto size = std::filesystem::file_size(inputPath);
                if (size > 15)
                {
                    throw std::runtime_error(
                        std::format(
                            "{}:{}:{}: The provided file {} is too large (> 15 bytes): {}", ass::filename, filepathToken.line, filepathToken.col, inputPath,
                            size
                        )
                    );
                }

                memPointer += static_cast<uint8_t>(size);

                i++; // Skip the next token as we already handled the input filepath

                if (i + 1 < tokens.size())
                {
                    auto nextToken = tokens[i + 1];
                    if (nextToken.type == TokenType::Newline)
                    {
                        i++;
                    }
                    else if (nextToken.type != TokenType::End)
                    {
                        throw std::runtime_error(std::format("{}:{}:{}: Unexpected token '{}'", ass::filename, nextToken.line, nextToken.col, nextToken.text));
                    }
                }

                continue;
            }
        }

        if (token.type == TokenType::LBracket && i + 1 < tokens.size() && tokens[i + 1].type != TokenType::Identifier && tokens[i + 1].text != "i" &&
            tokens[i + 1].text != "I")
        {
            throw std::runtime_error(std::format("{}:{}:{}: Unrecognized bracket expression", ass::filename, token.line, token.col));
        }

        if (token.type == TokenType::RBracket &&
            (i < 2 || (tokens[i - 1].type != TokenType::Identifier && tokens[i - 1].text != "i" && tokens[i - 1].text != "I") ||
             (tokens[i - 2].type != TokenType::LBracket)))
        {
            throw std::runtime_error(std::format("{}:{}:{}: Unrecognized bracket expression", ass::filename, token.line, token.col));
        }

        // If the previous token is not a newline and this one is, increment the pointer
        if (i > 0 && tokens[i - 1].type != TokenType::Newline && token.type == TokenType::Newline)
        {
            memPointer += 2;
        }
    }
}

Directive Parser::parseDirective(const Token& token, const std::vector<Token>& rawValues)
{
    if (equalsIgnoreCase(token.text, "DB") || equalsIgnoreCase(token.text, ".byte"))
    {
        std::vector<uint8_t> parsedValues{};

        for (const auto& v : rawValues)
        {
            uint8_t parsedValue{};

            auto err = std::from_chars(v.text.data() + 2, v.text.data() + v.text.size(), parsedValue, 16);

            if (err.ec != std::errc{})
            {
                throw std::runtime_error(
                    std::format("{}:{}:{}: Error when parsing directive argument: {}", ass::filename, v.line, v.col, std::make_error_code(err.ec).message())
                );
            }

            parsedValues.push_back(parsedValue);
        }

        Directive directive{
            .name = token.text,
            .values = std::move(parsedValues),
        };

        return directive;
    }

    if (equalsIgnoreCase(token.text, "INCBIN"))
    {
        if (rawValues.size() > 1 || rawValues.size() <= 0)
        {
            throw std::runtime_error(
                std::format(
                    "{}:{}:{}: Unexpected number of arguments provided for the INCBIN directive: {}", ass::filename, token.line, token.col, rawValues.size()
                )
            );
        }

        auto inputPath = std::string{rawValues[0].text};
        auto parsedValues = loadFileContentBytes(inputPath);

        Directive directive{
            .name = token.text,
            .values = std::move(parsedValues),
        };

        return directive;
    }

    throw std::runtime_error(std::format("{}:{}:{}: Unsupported directive found", ass::filename, token.line, token.col));
}

std::vector<std::variant<Instruction, Directive>> Parser::parseInstructions(const std::vector<Token>& tokens)
{
    std::vector<std::variant<Instruction, Directive>> out;

    Token mnem{};
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

            continue;
        }
        else if (token.type == TokenType::Identifier && firstIdentifier)
        {
            // It's a mnemonic
            mnem = token;
        }
        else if (
            token.type == TokenType::Identifier || token.type == TokenType::Number || token.type == TokenType::LBracket || token.type == TokenType::RBracket
        )
        {
            // It's an arg
            rawArgs.push_back(token);
        }
        else if ((token.type == TokenType::End || token.type == TokenType::Newline) && mnem.text != "")
        {
            std::variant<Instruction, Directive> parsed;

            if (isDirective(mnem))
            {
                parsed = parseDirective(mnem, rawArgs);
            }
            else
            {
                parsed = parseInstruction(mnem, rawArgs);
            }

            out.push_back(parsed);

            mnem = {};
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
    }

    return out;
}

uint8_t Parser::parseRegister(std::string_view registerString)
{
    if ((!registerString.starts_with("V") && !registerString.starts_with("v")))
    {
        throw std::runtime_error("Invalid register. Expected register name to start with 'V' and be in the Vx format.");
    }

    uint8_t regNumber;
    auto err = std::from_chars(registerString.data() + 1, registerString.data() + registerString.size(), regNumber, 16);

    if (err.ec != std::errc{})
    {
        throw std::runtime_error(std::format("Failed to parse register number: {} {}", std::make_error_code(err.ec).message(), err.ptr));
    }

    if (err.ptr != registerString.data() + registerString.size())
    {
        throw std::runtime_error(std::format("Found invalid character(s) in register number: {}", err.ptr));
    }

    if (regNumber > 15)
    {
        throw std::runtime_error(std::format("Register number must be between 0x0 and 0xF. Provided value: 0x{:X}", regNumber));
    }

    return regNumber;
}

uint8_t Parser::parseValue(std::string_view vString, uint8_t sourceValueBase)
{
    uint8_t value;
    auto err = std::from_chars(vString.data(), vString.data() + vString.size(), value, sourceValueBase);

    if (err.ec != std::errc{})
    {
        throw std::runtime_error(std::format("Failed to parse literal value: {}. Provided value: {}", std::make_error_code(err.ec).message(), vString));
    }

    return value;
}

uint16_t Parser::parseAddress(const std::string& aString, uint8_t sourceValueBase)
{
    uint16_t value;

    auto err = std::from_chars(aString.data(), aString.data() + aString.size(), value, sourceValueBase);

    if (err.ec != std::errc{})
    {
        throw std::runtime_error(std::format("Failed to parse address value: {}. Provided value: {}", std::make_error_code(err.ec).message(), aString));
    }

    if (value > 4095)
    {
        throw std::runtime_error(std::format("Failed to parse address value. Value provided is too large: {}", value));
    }

    return value;
}

auto Parser::findMatchingInstructionDefinition(const Token& mnem, const std::array<Operand, 3>& parsedOperandTypes, const std::vector<Token>& args)
{
    auto match = std::ranges::find_if(
        opTable,
        [&](const InstructionDefinition& instr)
        {
            if (!equalsIgnoreCase(mnem.text, instr.mnem))
            {
                return false;
            }

            // If there's brackets in the arg, e.g. [i], it inflates the operandCount
            // Decrement it to allow for proper comparison
            auto realSize = args.size();

            for (size_t i = 0; i < args.size(); i++)
            {
                if (args[i].type == TokenType::LBracket || args[i].type == TokenType::RBracket)
                {
                    realSize--;
                }
            }

            if (realSize != instr.operandCount)
            {
                return false;
            }

            bool compatible = true;
            for (size_t i = 0; i < instr.operandCount; i++)
            {
                if (!parsedOperandTypes[i].isCompatibleWith(instr.operands[i]))
                {
                    compatible = false;

                    break;
                }
            }

            return compatible;
        }
    );

    return match;
}

std::array<Operand, 3> Parser::parseOperandTypes(const std::vector<Token>& args)
{
    std::array<Operand, 3> parsedOperandTypes{};

    if (args.size() > 0)
    {
        size_t parsedOps = 0;

        for (size_t i = 0; i < args.size(); i++)
        {
            if (parsedOps >= 3)
            {
                throw std::runtime_error(std::format("{}:{}: Too many operands provided", ass::filename, args[0].line));
            }

            auto token = args[i];

            // [i] || [I]
            if (token.type == TokenType::LBracket && i + 2 < args.size() && args[i + 1].type == TokenType::Identifier &&
                (args[i + 1].text == "I" || args[i + 1].text == "i") && args[i + 2].type == TokenType::RBracket)
            {
                parsedOperandTypes[parsedOps] = {ArgType::I_MEM};

                i += 2; // Skip the next 2 tokens
            }

            else if (token.type == TokenType::Identifier && (token.text == "I" || token.text == "i"))
            {
                parsedOperandTypes[parsedOps] = {ArgType::I_REG};
            }

            else if (
                token.type == TokenType::Identifier && (token.text.starts_with("V") || token.text.starts_with("v")) && parsedOps > 0 &&
                parsedOperandTypes[parsedOps - 1].argType == ArgType::REGISTER_X
            )
            {
                parsedOperandTypes[parsedOps] = {ArgType::REGISTER_Y};
            }

            else if (token.type == TokenType::Identifier && (token.text.starts_with("V") || token.text.starts_with("v")))
            {
                parsedOperandTypes[parsedOps] = {ArgType::REGISTER_X};
            }

            else if (token.type == TokenType::Identifier && (token.text == "DT" || token.text == "dt"))
            {
                parsedOperandTypes[parsedOps] = {ArgType::DT};
            }

            else if (token.type == TokenType::Identifier && (token.text == "K" || token.text == "k"))
            {
                parsedOperandTypes[parsedOps] = {ArgType::KEY};
            }

            else if (token.type == TokenType::Identifier && (token.text == "ST" || token.text == "st"))
            {
                parsedOperandTypes[parsedOps] = {ArgType::ST};
            }

            else if (token.type == TokenType::Identifier && (token.text == "LF" || token.text == "lf" || token.text == "F" || token.text == "f"))
            {
                parsedOperandTypes[parsedOps] = {ArgType::FONT};
            }

            else if (token.type == TokenType::Identifier && (token.text == "B" || token.text == "b"))
            {
                parsedOperandTypes[parsedOps] = {ArgType::BCD};
            }

            else
            {
                parsedOperandTypes[parsedOps] = {ArgType::LITERAL};
            }

            parsedOps++;
        }
    }

    return parsedOperandTypes;
}

Instruction Parser::makeInstruction(const InstructionDefinition& def, const std::vector<Token>& args)
{
    auto out = Instruction{.def = &def};

    auto rawHex{def.hex};
    std::array<uint16_t, 3> parsedOpValues{};

    if (!def.operands.empty())
    {
        size_t currentOperand = 0;

        for (size_t i = 0; i < args.size(); i++)
        {
            if (args[i].type == TokenType::LBracket || args[i].type == TokenType::RBracket)
            {
                continue;
            }

            uint8_t sourceValueBase = 10;

            std::string str{args[i].text};
            if (str.starts_with("0x"))
            {
                sourceValueBase = 16;
                str = str.substr(2);
            }

            switch (def.operands[currentOperand].argType)
            {
            case ArgType::NONE:
                break;

            case ArgType::REGISTER_X:
            {
                uint8_t regNumber;

                try
                {
                    regNumber = parseRegister(str);
                }
                catch (const std::runtime_error& e)
                {
                    throw std::runtime_error(std::format("{}:{}:{}: {}", ass::filename, args[i].line, args[i].col, e.what()));
                }

                parsedOpValues[currentOperand] = regNumber;

                rawHex |= regNumber << 8;

                break;
            }

            case ArgType::REGISTER_Y:
            {
                uint8_t regNumber;

                try
                {
                    regNumber = parseRegister(str);
                }
                catch (const std::runtime_error& e)
                {
                    throw std::runtime_error(std::format("{}:{}:{}: {}", ass::filename, args[i].line, args[i].col, e.what()));
                }

                parsedOpValues[currentOperand] = regNumber;

                rawHex |= regNumber << 4;

                break;
            }

            case ArgType::LITERAL:
            {
                auto literalType = def.operands[i].literalType;

                if (!literalType.has_value())
                {
                    throw std::runtime_error(
                        std::format(
                            "{}:{}:{}: Cannot parse literal (arg specified as literal, but found no value for it)", ass::filename, args[i].line, args[i].col
                        )
                    );
                }

                if (literalType == LiteralType::VALUE_N)
                {
                    uint8_t value;

                    try
                    {
                        value = parseValue(str, sourceValueBase);
                    }
                    catch (const std::runtime_error& e)
                    {
                        throw std::runtime_error(std::format("{}:{}:{}: {}", ass::filename, args[i].line, args[i].col, e.what()));
                    }

                    if (value < 1 || value > 15)
                    {
                        throw std::runtime_error(
                            std::format(
                                "{}:{}:{}: Failed to parse value of type N. The value must be between 1 and 15. Provided value: {}", ass::filename,
                                args[i].line, args[i].col, value
                            )
                        );
                    }

                    parsedOpValues[currentOperand] = value;
                    rawHex |= value;

                    break;
                };

                if (literalType == LiteralType::VALUE_NN)
                {
                    uint8_t value;

                    try
                    {
                        value = parseValue(str, sourceValueBase);
                    }
                    catch (const std::runtime_error& e)
                    {
                        throw std::runtime_error(std::format("{}:{}:{}: {}", ass::filename, args[i].line, args[i].col, e.what()));
                    }

                    parsedOpValues[currentOperand] = value;
                    rawHex |= value;

                    break;
                };

                if (literalType == LiteralType::ADDRESS)
                {
                    // Labels can only be used as address placeholders
                    // Check if label map contains the literal and if so,
                    // turn the label into the assigned address
                    auto foundLabel = m_labelMemoryMap.find(str);
                    if (foundLabel != m_labelMemoryMap.end())
                    {
                        parsedOpValues[currentOperand] = foundLabel->second.addr;
                        rawHex |= foundLabel->second.addr;

                        break;
                    }

                    uint16_t value;

                    try
                    {
                        value = parseAddress(str, sourceValueBase);
                    }
                    catch (const std::runtime_error& e)
                    {
                        throw std::runtime_error(std::format("{}:{}:{}: {}", ass::filename, args[i].line, args[i].col, e.what()));
                    }

                    parsedOpValues[currentOperand] = value;
                    rawHex |= value;

                    break;
                };
            }

            // These args require no further value parsing
            // They are not included in the resulting hex value
            case ArgType::UNKNOWN:
            case ArgType::I_REG:
            case ArgType::DT:
            case ArgType::KEY:
            case ArgType::ST:
            case ArgType::FONT:
            case ArgType::BCD:
            case ArgType::I_MEM:
                break;
            }

            currentOperand++;
        }
    }

    out.encodedHex = rawHex;
    out.operandValues = parsedOpValues;

    return out;
}

Instruction Parser::parseInstruction(const Token& mnem, const std::vector<Token>& args)
{
    // Parse operand type for each arg
    // Needed so we know how to parse the values down the line
    auto parsedOperandTypes = parseOperandTypes(args);

    // Find matching instruction/op
    auto match = findMatchingInstructionDefinition(mnem, parsedOperandTypes, args);
    if (match == opTable.end())
    {
        std::string operands{};
        for (const auto& [idx, op] : args | std::ranges::views::enumerate)
        {
            operands += op.text;

            if (static_cast<unsigned long long>(idx) != args.size() - 1)
            {
                operands += ", ";
            }
        }

        throw std::runtime_error(
            std::format("{}:{}:{}: Invalid mnemonic + operand type combination: {} {}", ass::filename, mnem.line, mnem.col, mnem.text, operands)
        );
    }

    InstructionDefinition definition = *match;

    return makeInstruction(definition, args);
}
} // namespace ass
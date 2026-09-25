#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "instruction.h"
#include "lexer.h"

namespace ass
{

struct Label
{
    uint16_t addr;
    size_t line;
};

class Parser
{
  public:
    Parser();

    void parseLabels(const std::vector<Token>& tokens);
    std::vector<std::variant<Instruction, Directive>> parseInstructions(const std::vector<Token>& tokens);

  private:
    static bool isDirective(const Token& mnem);

    std::array<Operand, 3> parseOperandTypes(const std::vector<Token>& args);

    auto findMatchingInstructionDefinition(const Token& mnem, const std::array<Operand, 3>& parsedOperandTypes, const std::vector<Token>& args);

    Instruction makeInstruction(const InstructionDefinition* def, const std::vector<Token>& args);

    uint8_t parseRegister(std::string_view registerString);

    // Parses both N and NN values
    uint8_t parseValue(std::string_view vString, uint8_t sourceValueBase);

    uint16_t parseAddress(const std::string& aString, uint8_t sourceValueBase);

    Directive parseDirective(const Token& token, const std::vector<Token>& rawValues);

    std::unordered_map<std::string, Label> m_labelMemoryMap;

    Instruction parseInstruction(const Token& mnem, const std::vector<Token>& args);
};

} // namespace ass
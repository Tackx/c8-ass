#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
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
    std::vector<Instruction> parseInstructions(const std::vector<Token>& tokens);

  private:
    std::unordered_map<std::string, Label> m_labelMemoryMap;

    Instruction parseInstruction(std::string_view mnem, const std::vector<Token>& args);
};

} // namespace ass
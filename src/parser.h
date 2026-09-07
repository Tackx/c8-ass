#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "instruction.h"

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
    Parser(std::string_view lines);

    void parseLabels(std::string_view textContent);
    std::vector<Instruction> parseInstructions();

    // Static helpers
    static bool isWhitespace(char c);
    static bool isComment(char c);
    static bool isArgSeparator(char c);

  private:
    std::string_view m_lines;
    std::size_t m_lineNr;
    std::unordered_map<std::string, Label> m_labelMemoryMap;

    Instruction parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs);
};
} // namespace ass
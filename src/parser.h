#pragma once

#include <cstddef>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "instruction.h"

namespace ass
{
class Parser
{
  public:
    Parser(const std::string& inPath);

    std::optional<Instruction> parseLine();
    static bool isWhitespace(char c);
    static bool isComment(char c);
    static bool isArgSeparator(char c);

  private:
    std::ifstream m_fi;
    std::string m_line;
    std::size_t m_lineNr;

    Instruction parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs);
};
} // namespace ass
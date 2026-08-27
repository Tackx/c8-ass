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

  private:
    std::ifstream m_fi;
    std::string m_line;
    std::size_t m_lineNr = 1;

    Instruction parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs);
};
} // namespace ass
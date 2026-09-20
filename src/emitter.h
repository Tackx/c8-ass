#pragma once

#include <fstream>
#include <string>
#include <variant>
#include <vector>

#include "instruction.h"

namespace ass
{
class Emitter
{
  public:
    explicit Emitter(const std::string& outPath);

    void emit(const std::vector<std::variant<Instruction, Directive>>& instructions);

  private:
    std::ofstream m_fo;
};
} // namespace ass
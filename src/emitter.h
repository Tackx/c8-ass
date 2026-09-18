#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "instruction.h"

namespace ass
{
class Emitter
{
  public:
    explicit Emitter(const std::unordered_map<std::string, std::string>& flags);

    void emit(const std::vector<std::variant<Instruction, Directive>>& instructions);

  private:
    std::ofstream m_fo;
};
} // namespace ass
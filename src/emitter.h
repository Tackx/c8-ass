#pragma once

#include <fstream>
#include <string>

#include "instruction.h"

namespace ass
{
class Emitter
{
  public:
    explicit Emitter(const std::string& outPath);

    void emit(const Instruction& instruction);

  private:
    std::ofstream m_fo;
};
} // namespace ass
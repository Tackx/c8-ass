
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "emitter.h"
#include "instruction.h"

namespace ass
{

Emitter::Emitter(const std::string& outPath) : m_fo{outPath, std::ios_base::binary}
{
    if (!m_fo.is_open())
    {
        throw std::runtime_error("Cannot open output file");
    }
}

void Emitter::emit(const std::vector<std::variant<Instruction, Directive>>& inputs)
{
    for (const auto& input : inputs)
    {
        if (auto instr = std::get_if<Instruction>(&input))
        {
            m_fo.put(static_cast<char>(instr->encodedHex >> 8));
            m_fo.put(static_cast<char>(instr->encodedHex & 0xFF));
        }
        else if (auto dir = std::get_if<Directive>(&input))
        {
            for (const auto& binary : dir->values)
            {
                m_fo.put(static_cast<char>(binary));
            }
        }
    }
}

} // namespace ass
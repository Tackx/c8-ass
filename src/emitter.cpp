
#include <ios>
#include <stdexcept>
#include <string>

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

void Emitter::emit(const Instruction& instruction)
{
    m_fo.put(static_cast<char>(instruction.encodedHex >> 8));
    m_fo.put(static_cast<char>(instruction.encodedHex & 0xFF));
}

} // namespace ass
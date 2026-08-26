
#include <ios>
#include <stdexcept>
#include <string>

#include "emitter.h"
#include "instruction.h"

namespace ass
{

Emitter::Emitter(const std::string& outPath) : fo{outPath, std::ios_base::binary}
{
    if (!fo.is_open())
    {
        throw std::runtime_error("Cannot open output file");
    }
}

void Emitter::emit(const Instruction& instruction)
{
    fo.put(static_cast<char>(instruction.encodedHex >> 8));
    fo.put(static_cast<char>(instruction.encodedHex & 0xFF));
}

} // namespace ass
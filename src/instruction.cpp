#include "instruction.h"

namespace ass
{

bool Operand::isCompatibleWith(const Operand& other)
{
    if (this->literalType.has_value() && other.literalType.has_value())
    {
        return this->argType == other.argType && this->literalType == other.literalType;
    }

    return this->argType == other.argType;
}

} // namespace ass
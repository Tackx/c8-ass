#include "instruction.h"
#include "util.h"

namespace ass
{

bool operator==(const Instruction& a, const Instruction& b)
{
    return a.def == b.def && a.encodedHex == b.encodedHex && a.operandValues == b.operandValues;
}

bool operator==(const Directive& a, const Directive& b)
{
    return equalsIgnoreCase(a.name, b.name) && a.values == b.values;
}

bool Operand::isCompatibleWith(const Operand& other) const
{
    if (this->literalType.has_value() && other.literalType.has_value())
    {
        return this->argType == other.argType && this->literalType == other.literalType;
    }

    return this->argType == other.argType;
}

} // namespace ass
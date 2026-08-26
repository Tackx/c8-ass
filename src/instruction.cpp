#include "instruction.h"

namespace ass
{

// TODO: Implement as a method on the Operand struct?
bool isCompatible(Operand first, Operand second)
{
    if (first.literalType.has_value() && second.literalType.has_value())
    {
        return first.argType == second.argType && first.literalType == second.literalType;
    }

    return first.argType == second.argType;
}

} // namespace ass
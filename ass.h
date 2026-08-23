#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

enum class OperandKind
{
    NONE,
    REGISTER,
    VALUE_N,
    VALUE_NN,
    ADDRESS, // AKA NNN
    I_REG,
    V0,
    DT,
    KEY,
    ST,
    FONT,
    BCD,
    I_MEM
};

struct InstructionDefinition
{
    std::string_view mnem;
    std::uint16_t hex;
    std::uint8_t operandCount = 0;
    std::array<OperandKind, 3> operands;
};

struct Instruction
{
    const InstructionDefinition* def;
    std::array<uint16_t, 3> operandValues;
    bool label; // Should be set to true operandValues contain a label which needs to be resolved
};

constexpr std::array ops = {
    InstructionDefinition{.mnem = "CLS", .hex = 0x00E0},
    InstructionDefinition{.mnem = "RET", .hex = 0x00EE},
    InstructionDefinition{.mnem = "JP", .hex = 0x1000, .operandCount = 1, .operands = {OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "CALL", .hex = 0x2000, .operandCount = 1, .operands = {OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "SE", .hex = 0x3000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "SNE", .hex = 0x4000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "SE", .hex = 0x5000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0x6000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "ADD", .hex = 0x7000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "LD", .hex = 0x8000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "OR", .hex = 0x8001, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "AND", .hex = 0x8002, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "XOR", .hex = 0x8003, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "ADD", .hex = 0x8004, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SUB", .hex = 0x8005, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SHR", .hex = 0x8006, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SUBN", .hex = 0x8007, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SHL", .hex = 0x800E, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SNE", .hex = 0x9000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xA000, .operandCount = 2, .operands = {OperandKind::I_REG, OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "JP", .hex = 0xB000, .operandCount = 2, .operands = {OperandKind::V0, OperandKind::ADDRESS}},
    InstructionDefinition{.mnem = "RND", .hex = 0xC000, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::VALUE_NN}},
    InstructionDefinition{.mnem = "DRW", .hex = 0xD000, .operandCount = 3, .operands = {OperandKind::REGISTER, OperandKind::REGISTER, OperandKind::VALUE_N}},
    InstructionDefinition{.mnem = "SKP", .hex = 0xE09E, .operandCount = 1, .operands = {OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "SKNP", .hex = 0xE0A1, .operandCount = 1, .operands = {OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF007, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::DT}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF00A, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::KEY}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF015, .operandCount = 2, .operands = {OperandKind::DT, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF018, .operandCount = 2, .operands = {OperandKind::ST, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "ADD", .hex = 0xF01E, .operandCount = 2, .operands = {OperandKind::I_REG, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF029, .operandCount = 2, .operands = {OperandKind::FONT, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF033, .operandCount = 2, .operands = {OperandKind::BCD, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF055, .operandCount = 2, .operands = {OperandKind::I_MEM, OperandKind::REGISTER}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF065, .operandCount = 2, .operands = {OperandKind::REGISTER, OperandKind::I_MEM}},
};

struct Label
{
    uint16_t addr;
    size_t line;
};

inline std::unordered_map<std::string, Label> labelMemoryMap;

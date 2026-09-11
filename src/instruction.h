#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace ass
{

enum class ArgType
{
    NONE,
    UNKNOWN,
    // REGISTER,
    REGISTER_X,
    REGISTER_Y,
    LITERAL,
    I_REG,
    V0,
    DT,
    KEY,
    ST,
    FONT,
    BCD,
    I_MEM
};

enum class LiteralType
{
    VALUE_N,
    VALUE_NN,
    ADDRESS,
};

struct Operand
{
    ArgType argType{};
    std::optional<LiteralType> literalType{};
};

struct InstructionDefinition
{
    std::string_view mnem;
    std::uint16_t hex;
    std::uint8_t operandCount = 0;
    std::array<Operand, 3> operands{};
};

struct Instruction
{
    const InstructionDefinition& def;
    std::array<uint16_t, 3> operandValues;
    uint16_t encodedHex;
};

constexpr Operand V0{ArgType::V0};
constexpr Operand REG_X{ArgType::REGISTER_X};
constexpr Operand REG_Y{ArgType::REGISTER_Y};
constexpr Operand N{ArgType::LITERAL, LiteralType::VALUE_N};
constexpr Operand NN{ArgType::LITERAL, LiteralType::VALUE_NN};
constexpr Operand NNN{ArgType::LITERAL, LiteralType::ADDRESS};
constexpr Operand I_REG{ArgType::I_REG};
constexpr Operand I_MEM{ArgType::I_MEM};
// constexpr Operand V0{ArgType::V0};
constexpr Operand DT{ArgType::DT};
constexpr Operand ST{ArgType::ST};
constexpr Operand KEY{ArgType::KEY};
constexpr Operand FONT{ArgType::FONT};
constexpr Operand BCD{ArgType::BCD};

constexpr std::array opTable = {
    // ADD
    InstructionDefinition{.mnem = "ADD", .hex = 0x7000, .operandCount = 2, .operands = {REG_X, NN}},
    InstructionDefinition{.mnem = "ADD", .hex = 0x8004, .operandCount = 2, .operands = {REG_X, REG_Y}},
    InstructionDefinition{.mnem = "ADD", .hex = 0xF01E, .operandCount = 2, .operands = {I_REG, REG_X}},

    // AND
    InstructionDefinition{.mnem = "AND", .hex = 0x8002, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // CALL
    InstructionDefinition{.mnem = "CALL", .hex = 0x2000, .operandCount = 1, .operands = {NNN}},

    // CLS
    InstructionDefinition{.mnem = "CLS", .hex = 0x00E0},

    // DRW
    InstructionDefinition{.mnem = "DRW", .hex = 0xD000, .operandCount = 3, .operands = {REG_X, REG_Y, N}},

    // JP
    InstructionDefinition{.mnem = "JP", .hex = 0x1000, .operandCount = 1, .operands = {NNN}},
    // TODO: Unfuck this
    InstructionDefinition{.mnem = "JP", .hex = 0xB000, .operandCount = 2, .operands = {V0, NNN}},

    // LD
    InstructionDefinition{.mnem = "LD", .hex = 0x6000, .operandCount = 2, .operands = {REG_X, NN}},
    InstructionDefinition{.mnem = "LD", .hex = 0x8000, .operandCount = 2, .operands = {REG_X, REG_Y}},
    InstructionDefinition{.mnem = "LD", .hex = 0xA000, .operandCount = 2, .operands = {I_REG, NNN}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF007, .operandCount = 2, .operands = {REG_X, DT}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF00A, .operandCount = 2, .operands = {REG_X, KEY}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF015, .operandCount = 2, .operands = {DT, REG_X}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF018, .operandCount = 2, .operands = {ST, REG_X}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF029, .operandCount = 2, .operands = {FONT, REG_X}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF033, .operandCount = 2, .operands = {BCD, REG_X}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF055, .operandCount = 2, .operands = {I_MEM, REG_X}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF065, .operandCount = 2, .operands = {REG_X, I_MEM}},

    // OR
    InstructionDefinition{.mnem = "OR", .hex = 0x8001, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // RET
    InstructionDefinition{.mnem = "RET", .hex = 0x00EE},

    // RND
    InstructionDefinition{.mnem = "RND", .hex = 0xC000, .operandCount = 2, .operands = {REG_X, NN}},

    // SE
    InstructionDefinition{.mnem = "SE", .hex = 0x3000, .operandCount = 2, .operands = {REG_X, NN}},
    InstructionDefinition{.mnem = "SE", .hex = 0x5000, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // SHL
    InstructionDefinition{.mnem = "SHL", .hex = 0x800E, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // SHR
    InstructionDefinition{.mnem = "SHR", .hex = 0x8006, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // SKNP
    InstructionDefinition{.mnem = "SKNP", .hex = 0xE0A1, .operandCount = 1, .operands = {REG_X}},

    // SKP
    InstructionDefinition{.mnem = "SKP", .hex = 0xE09E, .operandCount = 1, .operands = {REG_X}},

    // SNE
    InstructionDefinition{.mnem = "SNE", .hex = 0x4000, .operandCount = 2, .operands = {REG_X, NN}},
    InstructionDefinition{.mnem = "SNE", .hex = 0x9000, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // SUB
    InstructionDefinition{.mnem = "SUB", .hex = 0x8005, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // SUBN
    InstructionDefinition{.mnem = "SUBN", .hex = 0x8007, .operandCount = 2, .operands = {REG_X, REG_Y}},

    // XOR
    InstructionDefinition{.mnem = "XOR", .hex = 0x8003, .operandCount = 2, .operands = {REG_X, REG_Y}},
};

Instruction parseInstruction(std::string_view mnem, std::vector<std::string_view> rawArgs, size_t lineNr);

bool isCompatible(Operand first, Operand second);
} // namespace ass

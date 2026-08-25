#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

enum class ArgType
{
    NONE,
    UNKNOWN,
    REGISTER,
    LITERAL,
    I_REG,
    // V0,
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
    ArgType argType;
    std::optional<LiteralType> literalType;
};

struct InstructionDefinition
{
    std::string_view mnem;
    std::uint16_t hex;
    std::uint8_t operandCount = 0;
    std::array<Operand, 3> operands;
};

struct Instruction
{
    const InstructionDefinition& def;
    std::array<uint16_t, 3> operandValues;
    uint16_t encodedHex;
};

constexpr Operand REG{ArgType::REGISTER};
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
    InstructionDefinition{.mnem = "ADD", .hex = 0x7000, .operandCount = 2, .operands = {REG, NN}},
    InstructionDefinition{.mnem = "ADD", .hex = 0x8004, .operandCount = 2, .operands = {REG, REG}},
    InstructionDefinition{.mnem = "ADD", .hex = 0xF01E, .operandCount = 2, .operands = {I_REG, REG}},

    // AND
    InstructionDefinition{.mnem = "AND", .hex = 0x8002, .operandCount = 2, .operands = {REG, REG}},

    // CALL
    InstructionDefinition{.mnem = "CALL", .hex = 0x2000, .operandCount = 1, .operands = {NNN}},

    // CLS
    InstructionDefinition{.mnem = "CLS", .hex = 0x00E0},

    // DRW
    InstructionDefinition{.mnem = "DRW", .hex = 0xD000, .operandCount = 3, .operands = {REG, REG, N}},

    // JP
    InstructionDefinition{.mnem = "JP", .hex = 0x1000, .operandCount = 1, .operands = {NNN}},
    InstructionDefinition{.mnem = "JP", .hex = 0xB000, .operandCount = 2, .operands = {REG, NNN}},

    // LD
    InstructionDefinition{.mnem = "LD", .hex = 0x6000, .operandCount = 2, .operands = {REG, NN}},
    InstructionDefinition{.mnem = "LD", .hex = 0x8000, .operandCount = 2, .operands = {REG, REG}},
    InstructionDefinition{.mnem = "LD", .hex = 0xA000, .operandCount = 2, .operands = {I_REG, NNN}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF007, .operandCount = 2, .operands = {REG, DT}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF00A, .operandCount = 2, .operands = {REG, KEY}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF015, .operandCount = 2, .operands = {DT, REG}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF018, .operandCount = 2, .operands = {ST, REG}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF029, .operandCount = 2, .operands = {FONT, REG}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF033, .operandCount = 2, .operands = {BCD, REG}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF055, .operandCount = 2, .operands = {I_MEM, REG}},
    InstructionDefinition{.mnem = "LD", .hex = 0xF065, .operandCount = 2, .operands = {REG, I_MEM}},

    // OR
    InstructionDefinition{.mnem = "OR", .hex = 0x8001, .operandCount = 2, .operands = {REG, REG}},

    // RET
    InstructionDefinition{.mnem = "RET", .hex = 0x00EE},

    // RND
    InstructionDefinition{.mnem = "RND", .hex = 0xC000, .operandCount = 2, .operands = {REG, NN}},

    // SE
    InstructionDefinition{.mnem = "SE", .hex = 0x3000, .operandCount = 2, .operands = {REG, NN}},
    InstructionDefinition{.mnem = "SE", .hex = 0x5000, .operandCount = 2, .operands = {REG, REG}},

    // SHL
    InstructionDefinition{.mnem = "SHL", .hex = 0x800E, .operandCount = 2, .operands = {REG, REG}},

    // SHR
    InstructionDefinition{.mnem = "SHR", .hex = 0x8006, .operandCount = 2, .operands = {REG, REG}},

    // SKNP
    InstructionDefinition{.mnem = "SKNP", .hex = 0xE0A1, .operandCount = 1, .operands = {REG}},

    // SKP
    InstructionDefinition{.mnem = "SKP", .hex = 0xE09E, .operandCount = 1, .operands = {REG}},

    // SNE
    InstructionDefinition{.mnem = "SNE", .hex = 0x4000, .operandCount = 2, .operands = {REG, NN}},
    InstructionDefinition{.mnem = "SNE", .hex = 0x9000, .operandCount = 2, .operands = {REG, REG}},

    // SUB
    InstructionDefinition{.mnem = "SUB", .hex = 0x8005, .operandCount = 2, .operands = {REG, REG}},

    // SUBN
    InstructionDefinition{.mnem = "SUBN", .hex = 0x8007, .operandCount = 2, .operands = {REG, REG}},

    // XOR
    InstructionDefinition{.mnem = "XOR", .hex = 0x8003, .operandCount = 2, .operands = {REG, REG}},
};

struct Label
{
    uint16_t addr;
    size_t line;
};

// TODO: Maybe refactor into a class along with the memory location counter and error list
inline std::unordered_map<std::string, Label> labelMemoryMap;

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <variant>
#include <vector>

#include "ass.h"
#include "instruction.h"
#include "lexer.h"
#include "loader.h"
#include "parser.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#define PORT_DUP _dup
#define PORT_DUP2 _dup2
#define PORT_CLOSE _close
#define PORT_OPEN(path) _open(path, _O_RDONLY | _O_BINARY)
#else
#include <fcntl.h>
#include <unistd.h>
#define PORT_DUP dup
#define PORT_DUP2 dup2
#define PORT_CLOSE close
#define PORT_OPEN(path) open(path, O_RDONLY)
#endif

constexpr int STDIN_FD = 0;

// ./ass -o output.ch8 ../tests/inputs/input.ass
TEST_CASE("Happy day: Output flag + filepath arg provided", "[E2E]")
{
    char* argv[] = {(char*)"ass", (char*)"-o", (char*)"output.ch8", (char*)"../tests/inputs/input.ass"};

    auto output = ass::assemble(std::size(argv), argv);

    REQUIRE(output == 0);
}

// ./ass ../tests/inputs/input.ass
TEST_CASE("Happy day: Only filepath arg provided", "[E2E]")
{
    char* argv[] = {(char*)"ass", (char*)"../tests/inputs/input.ass"};

    auto output = ass::assemble(std::size(argv), argv);

    REQUIRE(output == 0);
}

// ./ass < ../tests/inputs/input.ass
TEST_CASE("Happy day: Reading from a redirected stdin", "[E2E]")
{
    auto old_stdin = PORT_DUP(STDIN_FD);

    auto fd = PORT_OPEN("../tests/inputs/input.ass");
    REQUIRE(fd != -1);

    PORT_DUP2(fd, STDIN_FD);
    PORT_CLOSE(fd);

    char* argv[] = {(char*)"ass"};

    auto output = ass::assemble(std::size(argv), argv);

    PORT_DUP2(old_stdin, STDIN_FD);
    PORT_CLOSE(old_stdin);

    REQUIRE(output == 0);
}

// ./ass -o output.ch8 < ../tests/inputs/input.ass
TEST_CASE("Happy day: Reading from a redirected stdin with an output flag used", "[E2E]")
{
    auto old_stdin = PORT_DUP(STDIN_FD);

    auto fd = PORT_OPEN("../tests/inputs/input.ass");
    REQUIRE(fd != -1);

    PORT_DUP2(fd, STDIN_FD);
    PORT_CLOSE(fd);

    char* argv[] = {(char*)"ass", (char*)"-o", (char*)"specific_output.ch8"};

    auto output = ass::assemble(std::size(argv), argv);

    PORT_DUP2(old_stdin, STDIN_FD);
    PORT_CLOSE(old_stdin);

    REQUIRE(output == 0);
}

// ./ass -o ../tests/inputs/input.ass
TEST_CASE("Open output flag, only one follow-up argument. The input path cannot be determined.", "[E2E]")
{
    char* argv[] = {(char*)"ass", (char*)"-o", (char*)"../tests/inputs/input.ass"};

    auto output = ass::assemble(std::size(argv), argv);

    REQUIRE(output == 1);
}

// ./ass -o < ../tests/inputs/input.ass
TEST_CASE("Open outplug flag, redirected stdin", "[E2E]")
{
    auto old_stdin = PORT_DUP(STDIN_FD);

    auto fd = PORT_OPEN("../tests/inputs/input.ass");
    REQUIRE(fd != -1);

    PORT_DUP2(fd, STDIN_FD);
    PORT_CLOSE(fd);

    char* argv[] = {(char*)"ass", (char*)"-o"};

    auto output = ass::assemble(std::size(argv), argv);

    PORT_DUP2(old_stdin, STDIN_FD);
    PORT_CLOSE(old_stdin);

    REQUIRE(output == 1);
}

// ./ass
TEST_CASE("No args provided", "[E2E]")
{
    auto output = ass::assemble(0, nullptr);

    REQUIRE(output == 1);
}

TEST_CASE("Instructions + Directives", "[E2E]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD V0, 17\nLD V1, 4\nLD I, sprite_s\nDRW V0, V1, 5\n\nLD V0, 22\nLD I, sprite_r\nDRW V0, V1, 5\n\nsprite_s:\n    INCBIN "
                 "\"../tests/inputs/sprites/s.bin\" ; Quotes are ignored\n\nsprite_r:\nDB    0xE0, 0x90, 0xE0, 0x90, 0x90"};

    const auto tokens = l.produceTokens();
    REQUIRE(tokens.size() == 62);

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    std::vector<std::variant<ass::Instruction, ass::Directive>> expected{
        ass::Instruction{.def = &ass::opTable[9], .operandValues = {0, 17, 0}, .encodedHex = 0x6011},
        ass::Instruction{.def = &ass::opTable[9], .operandValues = {1, 4, 0}, .encodedHex = 0x6104},
        ass::Instruction{.def = &ass::opTable[11], .operandValues = {0, 526, 0}, .encodedHex = 0xA20E},
        ass::Instruction{.def = &ass::opTable[6], .operandValues = {0, 1, 5}, .encodedHex = 0xD015},
        ass::Instruction{.def = &ass::opTable[9], .operandValues = {0, 22, 0}, .encodedHex = 0x6016},
        ass::Instruction{.def = &ass::opTable[11], .operandValues = {0, 531, 0}, .encodedHex = 0xA213},
        ass::Instruction{.def = &ass::opTable[6], .operandValues = {0, 1, 5}, .encodedHex = 0xD015},
        ass::Directive{.name = "INCBIN", .values = std::vector<uint8_t>{0xF0, 0x80, 0xF0, 0x10, 0xF0}},
        ass::Directive{.name = "DB", .values = std::vector<uint8_t>{0xE0, 0x90, 0xE0, 0x90, 0x90}}
    };

    REQUIRE(output.size() == expected.size());

    for (const auto& [i, instr] : output | std::views::enumerate)
    {
        if (auto expectedValInstr = std::get_if<ass::Instruction>(&expected[(size_t)i]))
        {
            auto actualValueInstr = std::get<ass::Instruction>(instr);

            REQUIRE(actualValueInstr == *expectedValInstr);
        }

        else if (auto expectedValDir = std::get_if<ass::Directive>(&expected[(size_t)i]))
        {
            auto actualValueDir = std::get<ass::Directive>(instr);

            REQUIRE(actualValueDir == *expectedValDir);
        }

        else
        {
            throw std::runtime_error("Unsupported type in the list of expected values");
        }
    }
}
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <iterator>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "ass.h"
#include "fs.h"
#include "instruction.h"
#include "lexer.h"
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

auto happyInputPath = (std::filesystem::path{__FILE__}.remove_filename() / "inputs/input.ass").string();

// ./ass -o output.ch8 <path>/tests/inputs/input.ass
TEST_CASE("Happy day: Output flag + filepath arg provided", "[E2E]")
{
    char* argv[] = {(char*)"ass", (char*)"-o", (char*)"output.ch8", (char*)happyInputPath.data()};

    auto output = ass::assemble(std::size(argv), argv);

    REQUIRE(output == 0);
}

// ./ass <path>/tests/inputs/input.ass
TEST_CASE("Happy day: Only filepath arg provided", "[E2E]")
{
    char* argv[] = {(char*)"ass", (char*)happyInputPath.data()};

    auto output = ass::assemble(std::size(argv), argv);

    REQUIRE(output == 0);
}

// ./ass < <path>/tests/inputs/input.ass
TEST_CASE("Happy day: Reading from a redirected stdin", "[E2E]")
{
    auto old_stdin = PORT_DUP(STDIN_FD);

    auto fd = PORT_OPEN(happyInputPath.data());
    REQUIRE(fd != -1);

    PORT_DUP2(fd, STDIN_FD);
    PORT_CLOSE(fd);

    char* argv[] = {(char*)"ass"};

    auto output = ass::assemble(std::size(argv), argv);

    PORT_DUP2(old_stdin, STDIN_FD);
    PORT_CLOSE(old_stdin);

    REQUIRE(output == 0);
}

// ./ass -o output.ch8 < <path>/tests/inputs/input.ass
TEST_CASE("Happy day: Reading from a redirected stdin with an output flag used", "[E2E]")
{
    auto old_stdin = PORT_DUP(STDIN_FD);

    auto fd = PORT_OPEN(happyInputPath.data());
    REQUIRE(fd != -1);

    PORT_DUP2(fd, STDIN_FD);
    PORT_CLOSE(fd);

    char* argv[] = {(char*)"ass", (char*)"-o", (char*)"specific_output.ch8"};

    auto output = ass::assemble(std::size(argv), argv);

    PORT_DUP2(old_stdin, STDIN_FD);
    PORT_CLOSE(old_stdin);

    REQUIRE(output == 0);
}

// ./ass -o <path>/tests/inputs/input.ass
TEST_CASE("Open output flag, only one follow-up argument. The input path cannot be determined.", "[E2E]")
{
    char* argv[] = {(char*)"ass", (char*)"-o", (char*)happyInputPath.data()};

    auto output = ass::assemble(std::size(argv), argv);

    REQUIRE(output == 1);
}

// ./ass -o < <path>/tests/inputs/input.ass
TEST_CASE("Open outplug flag, redirected stdin", "[E2E]")
{
    auto old_stdin = PORT_DUP(STDIN_FD);

    auto fd = PORT_OPEN(happyInputPath.data());
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
    ass::inputPathAbsolute = std::filesystem::path{__FILE__}.remove_filename();

    ass::Lexer l{"LD V0, 17\nLD V1, 4\nLD I, sprite_s\nDRW V0, V1, 5\n\nLD V0, 22\nLD I, sprite_r\nDRW V0, V1, 5\n\nsprite_s:\n    INCBIN "
                 "\"inputs/sprites/s.bin\" ; Quotes are ignored\n\nsprite_r:\nDB    0xE0, 0x90, 0xE0, 0x90, 0x90"};

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

TEST_CASE("Resulting ROM too large (> 4096 bytes)", "[E2E]")
{
    auto inputPath = (std::filesystem::path{__FILE__}.remove_filename() / "inputs/input_too_large.ass").string();

    std::string expected{"input_too_large.ass:221:19904: Error while incrementing memory pointer: Maximum ROM size reached"};
    std::string actual{};

    try
    {
        auto fileContent = ass::loadFileContentString(inputPath);

        ass::Lexer lexer{fileContent};
        ass::Parser parser{};

        auto tokens = lexer.produceTokens();

        parser.parseLabels(tokens);
    }
    catch (const std::exception& e)
    {
        actual = e.what();
    }

    std::println("Expected: {}", expected);
    std::println("Actual: {}", actual);

    REQUIRE(expected.compare(actual) == 0);
}

TEST_CASE("Check resulting output file bytes", "[E2E]")
{
    auto inputPath = (std::filesystem::path{__FILE__}.remove_filename() / "inputs/cls.ass").string();

    auto outputPath = (std::filesystem::path{__FILE__}.remove_filename() / "output_cls.ch8").string();

    char* argv[] = {(char*)"ass", (char*)"-o", (char*)outputPath.c_str(), (char*)inputPath.data()};

    ass::assemble(std::size(argv), argv);

    std::vector<uint8_t> expected{0x00, 0xE0};
    std::vector<uint8_t> actual = ass::loadFileContentBytes(outputPath);

    REQUIRE(expected == actual);
}

// TODO: Use ensurePathExists() or w/e to create the output directory for test outputs
TEST_CASE("Check resulting output file bytes - complex file", "[E2E]")
{
    auto inputPath = (std::filesystem::path{__FILE__}.remove_filename() / "inputs/starting_soon.ass").string();

    auto outputPath = (std::filesystem::path{__FILE__}.remove_filename() / "output_starting_soon.ch8").string();

    char* argv[] = {(char*)"ass", (char*)"-o", (char*)outputPath.c_str(), (char*)inputPath.data()};

    ass::assemble(std::size(argv), argv);

    std::vector<uint8_t> expected{0x00, 0xe0, 0x60, 0x11, 0x61, 0x04, 0xa2, 0xed, 0xd0, 0x15, 0x60, 0x16, 0xa2, 0xf2, 0xd0, 0x15, 0x60, 0x1a, 0xa2, 0xe8, 0xd0,
                                  0x15, 0x60, 0x1f, 0x62, 0x0e, 0xf2, 0x29, 0xd0, 0x15, 0x60, 0x24, 0x62, 0x0a, 0xf2, 0x29, 0xd0, 0x15, 0x60, 0x29, 0xa2, 0xf7,
                                  0xd0, 0x15, 0x61, 0x0a, 0x60, 0x0f, 0xa2, 0xed, 0xd0, 0x15, 0x60, 0x14, 0xa2, 0xf2, 0xd0, 0x15, 0x60, 0x18, 0x62, 0x0a, 0xf2,
                                  0x29, 0xd0, 0x15, 0x60, 0x1d, 0xa2, 0xe8, 0xd0, 0x15, 0x60, 0x22, 0xa2, 0xf2, 0xd0, 0x15, 0x60, 0x26, 0xa2, 0xfc, 0xd0, 0x15,
                                  0x60, 0x28, 0xa3, 0x01, 0xd0, 0x15, 0x60, 0x2d, 0xa3, 0x06, 0xd0, 0x15, 0x61, 0x10, 0x60, 0x16, 0xa2, 0xed, 0xd0, 0x15, 0x60,
                                  0x1b, 0x62, 0x00, 0xf2, 0x29, 0xd0, 0x15, 0x60, 0x20, 0x62, 0x00, 0xf2, 0x29, 0xd0, 0x15, 0x60, 0x25, 0xa3, 0x01, 0xd0, 0x15,
                                  0x69, 0x17, 0x6a, 0x1c, 0x60, 0x20, 0x61, 0x25, 0x62, 0x17, 0x67, 0x04, 0x63, 0x05, 0x64, 0x09, 0xf7, 0x29, 0xd9, 0x25, 0xa3,
                                  0x0b, 0xda, 0x25, 0xf3, 0x29, 0xd0, 0x25, 0xf4, 0x29, 0xd1, 0x25, 0x65, 0x3c, 0xf5, 0x15, 0xf5, 0x07, 0x35, 0x00, 0x12, 0xa2,
                                  0xf7, 0x29, 0xd9, 0x25, 0xa3, 0x0b, 0xda, 0x25, 0xf4, 0x29, 0xd1, 0x25, 0xf3, 0x29, 0xd0, 0x25, 0x46, 0x01, 0x12, 0xc6, 0x34,
                                  0x00, 0x74, 0xff, 0x44, 0x00, 0x66, 0x01, 0x12, 0x8e, 0x66, 0x00, 0x48, 0x01, 0x12, 0xd8, 0x33, 0x00, 0x73, 0xff, 0x64, 0x09,
                                  0x43, 0x00, 0x68, 0x01, 0x12, 0x8e, 0x68, 0x00, 0x47, 0x00, 0x12, 0x7e, 0x37, 0x00, 0x77, 0xff, 0x63, 0x05, 0x64, 0x09, 0x12,
                                  0x8e, 0xe0, 0x90, 0xe0, 0x90, 0x90, 0xf0, 0x80, 0xf0, 0x10, 0xf0, 0xe0, 0x40, 0x40, 0x40, 0x40, 0x88, 0xd8, 0xa8, 0x88, 0x88,
                                  0x80, 0x80, 0x80, 0x80, 0x80, 0x90, 0x90, 0xd0, 0xb0, 0x90, 0xf0, 0x80, 0xb0, 0x90, 0xf0, 0x00, 0x40, 0x00, 0x40, 0x00};

    std::vector<uint8_t> actual = ass::loadFileContentBytes(outputPath);

    REQUIRE(expected == actual);
}

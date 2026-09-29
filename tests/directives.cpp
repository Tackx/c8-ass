#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <filesystem>
#include <variant>
#include <vector>

#include "instruction.h"
#include "lexer.h"
#include "loader.h"
#include "parser.h"

TEST_CASE("DB", "[directives]")
{
    ass::filename = "<test>";

    ass::Lexer l{"DB 0xE0, 0x90, 0xE0, 0x90, 0x90"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto dir = output[0];

    auto val = std::get_if<ass::Directive>(&dir);

    REQUIRE(val != nullptr);
    REQUIRE(val->name == "DB");

    std::vector<uint8_t> expected{0xE0, 0x90, 0xE0, 0x90, 0x90};
    REQUIRE(val->values == expected);
}

TEST_CASE("DB with decimal and hex numbers mixed", "[directives]")
{
    ass::filename = "<test>";

    ass::Lexer l{"DB 15, 0x90, 0xE0, 0x90, 255"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto dir = output[0];

    auto val = std::get_if<ass::Directive>(&dir);

    REQUIRE(val != nullptr);
    REQUIRE(val->name == "DB");

    std::vector<uint8_t> expected{0x0F, 0x90, 0xE0, 0x90, 0xFF};
    REQUIRE(val->values == expected);
}

TEST_CASE("DB but lowercase", "[directives]")
{
    ass::filename = "<test>";

    ass::Lexer l{"db 0xE0, 0x90, 0xE0, 0x90, 0x91"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto dir = output[0];

    auto val = std::get_if<ass::Directive>(&dir);

    REQUIRE(val != nullptr);
    REQUIRE(val->name == "db");

    std::vector<uint8_t> expected{0xE0, 0x90, 0xE0, 0x90, 0x91};
    REQUIRE(val->values == expected);
}

TEST_CASE(".byte", "[directives]")
{
    ass::filename = "<test>";

    ass::Lexer l{".byte 0xEF, 0x90, 0xE0, 0x90, 0x90"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto dir = output[0];

    auto val = std::get_if<ass::Directive>(&dir);

    REQUIRE(val != nullptr);
    REQUIRE(val->name == ".byte");

    std::vector<uint8_t> expected{0xEF, 0x90, 0xE0, 0x90, 0x90};
    REQUIRE(val->values == expected);
}

TEST_CASE("INCBIN", "[directives]")
{
    ass::filename = "<test>";
    ass::inputPathAbsolute = std::filesystem::path{__FILE__}.remove_filename();

    ass::Lexer l{"INCBIN inputs/sprites/s.bin"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto dir = output[0];

    auto val = std::get_if<ass::Directive>(&dir);

    REQUIRE(val != nullptr);
    REQUIRE(val->name == "INCBIN");

    std::vector<uint8_t> expected{0xF0, 0x80, 0xF0, 0x10, 0xF0};
    REQUIRE(val->values == expected);
}

TEST_CASE("INCBIN with quotes", "[directives]")
{
    ass::filename = "<test>";
    ass::inputPathAbsolute = std::filesystem::path{__FILE__}.remove_filename();

    ass::Lexer l{"INCBIN \"inputs/sprites/s.bin\""};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto dir = output[0];

    auto val = std::get_if<ass::Directive>(&dir);

    REQUIRE(val != nullptr);
    REQUIRE(val->name == "INCBIN");

    std::vector<uint8_t> expected{0xF0, 0x80, 0xF0, 0x10, 0xF0};
    REQUIRE(val->values == expected);
}
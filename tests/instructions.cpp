#include <catch2/catch_test_macros.hpp>
#include <variant>

#include "instruction.h"
#include "lexer.h"
#include "loader.h"
#include "parser.h"

TEST_CASE("CLS", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"CLS"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x00E0);
}

TEST_CASE("CLS with a newline", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"CLS\n"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x00E0);
}

TEST_CASE("ADD 7xnn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"ADD V0, 123"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x707B);
}

TEST_CASE("ADD 8xy4", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"ADD V0, V1"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x8014);
}

TEST_CASE("ADD Fx1E", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"ADD I, V1"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xF11E);
}

TEST_CASE("AND 8xy2", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"AND V1, V4"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x8142);
}

TEST_CASE("CALL 2nnn hex", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"CALL 0x123"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x2123);
}

TEST_CASE("CALL 2nnn decimal", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"CALL 123"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x207B);
}

TEST_CASE("DRW Dxyn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"DRW V0, V1, 5"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xD015);
}

TEST_CASE("JP 1nnn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"JP 0x123"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x1123);
}

TEST_CASE("JP0 1nnn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"JP0 0x123"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xB123);
}

TEST_CASE("LD 6xnn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD V0, 0xFF"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x60FF);
}

TEST_CASE("LD 8xy0", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD V0, V1"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x8010);
}

TEST_CASE("LD Annn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD I, 0x123"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xA123);
}

TEST_CASE("LD Fx07", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD V0, DT"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xF007);
}

TEST_CASE("LD Fx0A", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD V0, K"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xF00A);
}

TEST_CASE("LD Fx15", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD DT, V7"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xF715);
}

TEST_CASE("LD Fx18", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD ST, VF"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xFF18);
}

TEST_CASE("LD Fx29", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD F, VA"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xFA29);
}

TEST_CASE("LD Fx33", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD B, V5"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xF533);
}

TEST_CASE("LD Fx55", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD [I], V8"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xF855);
}

TEST_CASE("LD Fx65", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"LD v3, [i]"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xF365);
}

TEST_CASE("OR 8xy1", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"OR V1, V9"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x8191);
}

TEST_CASE("RET 00EE", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"RET"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x00EE);
}

TEST_CASE("RND Cxnn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"RND V2, 0xAF"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xC2AF);
}

TEST_CASE("SE 3xnn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SE V0, 0x1E"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x301E);
}

TEST_CASE("SE 5xy0", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SE V0, V1"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x5010);
}

TEST_CASE("SHL 8xyE", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SHL V0, V5"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x805E);
}

TEST_CASE("SHR 8xy6", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SHR V0, V5"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x8056);
}

TEST_CASE("SKNP ExA1", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SKNP VB"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xEBA1);
}

TEST_CASE("SKP Ex9E", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SKP VE"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0xEE9E);
}

TEST_CASE("SNE 4xnn", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SNE V6, 0xAB"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x46AB);
}

TEST_CASE("SNE 9xy0", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SNE V0, VF"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x90F0);
}

TEST_CASE("SUB 8xy5", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SUB VF, VA"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x8FA5);
}

TEST_CASE("SUBN 8xy7", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"SUBN V9, VA"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x89A7);
}

TEST_CASE("XOR 8xy3", "[parser]")
{
    ass::filename = "<test>";

    ass::Lexer l{"XOR VC, VD"};
    const auto tokens = l.produceTokens();

    ass::Parser p{};
    p.parseLabels(tokens);
    auto output = p.parseInstructions(tokens);

    REQUIRE(output.size() == 1);

    auto instr = output[0];
    REQUIRE(std::holds_alternative<ass::Instruction>(instr));

    auto val = std::get<ass::Instruction>(instr);

    REQUIRE(val.encodedHex == 0x8CD3);
}

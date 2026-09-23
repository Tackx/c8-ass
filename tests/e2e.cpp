#include <catch2/catch_test_macros.hpp>
#include <iterator>

#include "ass.h"

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
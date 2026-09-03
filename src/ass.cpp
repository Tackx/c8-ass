#include <exception>
#include <format>
#include <fstream>
#include <ios>
#include <optional>
#include <print>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "ass.h"
#include "emitter.h"
#include "label.h"
#include "parser.h"

using namespace ass;

int emitCode(char** args)
{
    Parser parser{args[1]};
    Emitter emitter{args[2]};

    while (true)
    {
        auto parsedInstruction = parser.parseLine();

        if (!parsedInstruction.has_value())
        {
            break;
        }

        emitter.emit(*parsedInstruction);
    }

    return 0;
}

int ass::assemble(int argc, char** args)
{
    static constexpr std::string_view USAGE_MSG = "Usage: ass INPUT OUTPUT";

    // TODO: Enable reading from a redirected stream
    if (argc <= 2)
    {

        if (argc <= 1)
        {
            std::println("Missing input path.");
        }

        std::println("Missing output path.");

        std::println("{}", USAGE_MSG);

        return 1;
    }

    std::stringstream buffer;
    {
        std::ifstream t(args[1], std::ios::binary);
        if (!t)
        {
            throw std::runtime_error("Failed to open file.");
        }

        buffer << t.rdbuf();
    }

    auto fileContent = std::move(buffer).str();

    try
    {
        int r = 0;
        // First pass, which only parses and stores labels + their memory addresses
        // for substitution in second pass
        r = parseLabels(fileContent);
        if (r != 0)
        {
            return r;
        }

        // Second pass
        r = emitCode(args);

        return r;
    }
    catch (const std::exception& e)
    {
        std::println("Exception caught: {}", e.what());

        return 1;
    }
}

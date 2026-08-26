#include <format>
#include <optional>
#include <print>
#include <string>
#include <string_view>

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

    int r = 0;

    // First pass, which only parses and stores labels + their memory addresses
    // for substitution in second pass
    r = parseLabels(args);
    if (r != 0)
    {
        return r;
    }

    // Second pass
    r = emitCode(args);

    return r;
}

#include <exception>
#include <format>
#include <fstream>
#include <ios>
#include <print>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "ass.h"
#include "emitter.h"
#include "parser.h"

using namespace ass;

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
        Parser parser{fileContent};
        Emitter emitter{args[2]};

        // First pass, which only parses and stores labels + their memory addresses
        // for substitution in second pass
        parser.parseLabels(fileContent);

        // Second pass
        auto parsedInstructions = parser.parseInstructions();

        emitter.emit(parsedInstructions);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::println("Exception caught: {}", e.what());

        return 1;
    }
}

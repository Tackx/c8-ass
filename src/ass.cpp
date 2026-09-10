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
#include "lexer.h"
#include "parser.h"

namespace ass
{

int assemble(int argc, char** args)
{
    static constexpr std::string_view USAGE_MSG = "Usage: ass INPUT OUTPUT";

    // TODO: Enable reading from a redirected stream
    // TODO2: Enable specifying output path as a flag (-o)

    if (argc <= 2)
    {
        if (argc <= 1)
        {
            // TODO: Try to read from stdin?
            std::println("Missing input path.");
        }

        std::println("{}", USAGE_MSG);

        return 1;
    }

    try
    {
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
        Lexer lexer{fileContent};
        Parser parser{fileContent};
        Emitter emitter{args[2]};

        auto tokens = lexer.getTokens();

        // First pass, which only parses and stores labels + their memory addresses
        // for substitution in second pass
        parser.parseLabels(tokens);

        // Second pass
        auto parsedInstructions = parser.parseInstructions(tokens);

        emitter.emit(parsedInstructions);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::println("Exception caught: {}", e.what());

        return 1;
    }
}

} // namespace ass

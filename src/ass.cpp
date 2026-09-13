#include <exception>
#include <format>
#include <print>
#include <string>

#include "ass.h"
#include "emitter.h"
#include "flags.h"
#include "lexer.h"
#include "loader.h"
#include "parser.h"

namespace ass
{

int assemble(int argc, char** argv)
{
    static constexpr auto USAGE_MSG = "Usage: ass [FLAGS]... INPUT\n-o  Specifies the output path and filename. Default = current directory + 'output.ch8'\n";

    // TODO: Move to Emitter
    std::string outPath{"./output.ch8"};

    try
    {
        auto [flags, inPath] = getFlags(argc, argv);

        auto fileContent = loadFileContent(inPath);

        Lexer lexer{fileContent};
        Parser parser{};
        Emitter emitter{outPath};

        auto tokens = lexer.getTokens();

        // First pass, which only parses and stores labels + their memory addresses
        // for substitution in second pass
        parser.parseLabels(tokens);

        // Second pass
        auto parsedInstructions = parser.parseInstructions(tokens);

        emitter.emit(parsedInstructions);

        return 0;
    }
    catch (const NoInputException& e)
    {
        std::println("Exception caught: {}\n", e.what());
        std::println("{}", USAGE_MSG);

        return 1;
    }
    catch (const std::exception& e)
    {
        std::println("Exception caught: {}", e.what());

        return 1;
    }
}

} // namespace ass

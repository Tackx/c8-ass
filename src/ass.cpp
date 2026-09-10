#include <cstdio>
#include <exception>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <print>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include "ass.h"
#include "emitter.h"
#include "lexer.h"
#include "parser.h"

namespace ass
{

int assemble(int argc, char** args)
{
    static constexpr auto USAGE_MSG = "Usage: ass INPUT OUTPUT";
    static constexpr auto DEFAULT_OUT_PATH = "./output.ch8";

    // TODO: Enable specifying output path as a flag (-o)
    try
    {
        std::stringstream buffer;
        std::string fileContent;

        if (argc <= 1 && isatty(fileno(stdin)))
        {
            std::println("{}", USAGE_MSG);

            throw std::runtime_error("No input file path provided.");
        }

        else if (argc >= 2)
        {
            // Try to load input from args[1]
            {
                std::ifstream t(args[1], std::ios::binary);
                if (!t)
                {
                    throw std::runtime_error("Failed to open file.");
                }

                buffer << t.rdbuf();
            }
        }

        else if (argc >= 1)
        {
            // Try to load input from stdin
            buffer << std::cin.rdbuf();
        }

        fileContent = std::move(buffer).str();

        Lexer lexer{fileContent};
        Parser parser{fileContent};
        Emitter emitter{DEFAULT_OUT_PATH};

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

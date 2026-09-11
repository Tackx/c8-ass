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
#include <string_view>
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

    std::string inPath{};
    std::string outPath{"./output.ch8"};

    // Arg count can be:
    // - 1 (executable name) -- ass
    // - 2 (exec. name + input path) -- ass input.ass
    // - 3 (exec. name + optional output flag) -- ass -o foo.ch8 < input.ass
    // - 4 (exec. name + optional output flag + input path) -- ass -o foo.ch8 input.ass
    try
    {
        std::stringstream buffer;
        std::string fileContent;

        bool isFileInput = !isatty(fileno(stdin));

        // Parse flags
        size_t lastFlagValueIndex{0};
        for (size_t i = 1; i < (size_t)argc; i++)
        {
            std::string_view arg = args[i];

            if (arg.starts_with("-"))
            {
                // It's a flag
                if (arg != "-o")
                {
                    throw std::runtime_error(std::format("Unsupported flag {}", arg));
                }

                if (i + 1 >= (size_t)argc)
                {
                    throw std::runtime_error("Missing value for flag");
                }

                continue;
            }

            if (argc > 2)
            {
                // It's an argument (value of a flag)
                // For now, there's only 1 supported flag
                outPath = arg;
                lastFlagValueIndex = i;

                if (i + 1 < (size_t)argc)
                {
                    // If the next argument is not a flag, break out of parsing
                    std::string_view nextArg = args[i + 1];

                    if (!nextArg.starts_with("-"))
                    {
                        break;
                    }
                }
            }
        }

        if (lastFlagValueIndex + 1 < (size_t)argc)
        {
            std::string_view value = args[lastFlagValueIndex + 1];

            if (!value.starts_with("-"))
            {
                inPath = value;
            }
        }

        if (!inPath.empty())
        {
            std::ifstream t(inPath, std::ios::binary);
            if (!t)
            {
                throw std::runtime_error("Failed to open file");
            }

            buffer << t.rdbuf();
        }
        else if (isFileInput)
        {
            // Try to load input from stdin
            buffer << std::cin.rdbuf();
        }
        else
        {
            std::println("{}", USAGE_MSG);

            throw std::runtime_error("No input specified");
        }

        fileContent = std::move(buffer).str();

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
    catch (const std::exception& e)
    {
        std::println("Exception caught: {}", e.what());

        return 1;
    }
}

} // namespace ass

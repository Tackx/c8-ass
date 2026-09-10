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

        switch (argc)
        {
        case 2:
        {
            // TODO: Validate the arg is not a flag (since it does not make sense to only have a dangling flag)

            // Try to load input from args[1]
            inPath = args[1];

            break;
        }

        case 3:
        {
            // TODO: We need to make sure that the second arg is a (supported) flag and the third arg is a filepath and that we have a file input stream
            std::string_view flag{args[1]};
            std::string_view arg{args[2]};

            if (flag != "-o")
            {
                throw std::runtime_error(std::format("Unsupported flag {}", flag));
            }

            outPath = arg;

            break;
        }

        case 4:
        {
            std::string_view flag{args[1]};
            std::string_view arg1{args[2]};
            std::string_view arg2{args[3]};

            if (flag != "-o")
            {
                throw std::runtime_error(std::format("Unsupported flag {}", flag));
            }

            outPath = arg1;
            inPath = arg2;

            break;
        }

        default:
            // Throw too many flags?
            break;
        }

        if (!inPath.empty())
        {
            std::ifstream t(inPath, std::ios::binary);
            if (!t)
            {
                throw std::runtime_error("Failed to open file.");
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

            throw std::runtime_error("No input specified.");
        }

        fileContent = std::move(buffer).str();

        Lexer lexer{fileContent};
        Parser parser{fileContent};
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

#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "flags.h"

Result getFlags(int argc, char** argv)
{
    std::unordered_map<std::string, std::string> flags{};

    // Parse flags
    size_t lastFlagValueIndex{0};
    std::string currentFlag{};
    std::string inPath = {};

    for (size_t i = 1; i < (size_t)argc; i++)
    {
        std::string_view arg = argv[i];

        if (arg.starts_with("-"))
        {
            // It's a flag
            currentFlag = arg;

            if (arg != "-o")
            {
                throw std::runtime_error(std::format("Unsupported flag {}", arg));
            }

            if (i + 1 >= (size_t)argc)
            {
                throw std::runtime_error("Missing value for flag");
            }

            if (arg.starts_with('-'))
            {
                arg.remove_prefix(1);
            }

            flags[currentFlag] = std::string{};

            continue;
        }

        if (argc > 2)
        {
            // It's an argument (value of a flag)
            lastFlagValueIndex = i;

            flags[currentFlag] = arg;
            currentFlag = std::string{};

            if (i + 1 < (size_t)argc)
            {
                // If the next argument is not a flag, break out of parsing
                std::string_view nextArg = argv[i + 1];

                if (!nextArg.starts_with("-"))
                {
                    break;
                }
            }
        }
    }

    if (lastFlagValueIndex + 1 < (size_t)argc)
    {
        std::string_view value = argv[lastFlagValueIndex + 1];

        if (!value.starts_with("-"))
        {
            inPath = value;
        }
    }

    return {std::move(flags), std::move(inPath)};
}
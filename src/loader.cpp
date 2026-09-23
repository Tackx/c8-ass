
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include "loader.h"

namespace ass
{

std::string loadFileContent(const std::string& inputPath)
{
    std::stringstream buffer;
    bool isFileInput = !isatty(fileno(stdin));

    if (inputPath != "")
    {
        ass::filename = inputPath.substr(inputPath.find_last_of("/\\") + 1);

        std::ifstream t(inputPath, std::ios::binary);
        if (!t)
        {
            throw std::runtime_error("Failed to open file");
        }

        buffer << t.rdbuf();

        return buffer.str();
    }

    if (isFileInput)
    {
        ass::filename = "[STDIN]";
        // Try to load input from stdin
        buffer << std::cin.rdbuf();

        return buffer.str();
    }

    throw NoInputException("No input specified");
}

void ensureFilepathExists(std::string& filepath)
{
    auto path = std::filesystem::path{filepath};
    path.remove_filename();

    if (path.string().size() > 0)
    {
        std::filesystem::create_directories(path);
    }
    else
    {
        filepath = "./" + filepath;
    }
}
} // namespace ass

#include <cstdio>
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
        // Try to load input from stdin
        buffer << std::cin.rdbuf();

        return buffer.str();
    }

    throw NoInputException("No input specified");
}
} // namespace ass
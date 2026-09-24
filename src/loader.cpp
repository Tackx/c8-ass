
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

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

std::string loadFileContentString(const std::string& inputPath)
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

std::string& removeQuotes(std::string& inputPath)
{
    if (inputPath.starts_with("\""))
    {
        inputPath = inputPath.substr(1);
    }

    if (inputPath.ends_with("\""))
    {
        inputPath = inputPath.substr(0, inputPath.size() - 1);
    }

    return inputPath;
}

std::vector<uint8_t> loadFileContentBytes(std::string& inputPath)
{
    auto trimmed = removeQuotes(inputPath);

    auto fileExists = std::filesystem::exists(trimmed);
    if (!fileExists)
    {
        throw std::runtime_error(std::format("The provided filepath does not exist"));
    }

    std::ifstream inputStream{trimmed, std::ios_base::binary};

    std::vector<uint8_t> bytes{(std::istreambuf_iterator<char>{inputStream}), (std::istreambuf_iterator<char>{})};

    inputStream.close();

    return bytes;
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
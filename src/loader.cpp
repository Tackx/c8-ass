
#include <cerrno>
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
#include <system_error>
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

bool isFileInput()
{
    return !isatty(fileno(stdin));
}

std::string loadFileContentString(const std::filesystem::path& inputPath)
{
    std::stringstream buffer;

    if (inputPath != "")
    {
        ass::filename = inputPath.filename().string();
        ass::inputPathAbsolute = std::filesystem::absolute(inputPath).remove_filename();

        std::ifstream t(inputPath, std::ios::binary);
        if (!t)
        {
            throw std::system_error(errno, std::system_category(), std::format("Failed to open file {}", ass::filename));
        }

        buffer << t.rdbuf();

        return buffer.str();
    }

    if (isFileInput())
    {
        ass::filename = "[STDIN]";
        ass::inputPathAbsolute = std::filesystem::current_path();

        // Try to load input from stdin
        buffer << std::cin.rdbuf();

        return buffer.str();
    }

    throw NoInputException("No input specified");
}

std::vector<uint8_t> loadFileContentBytes(const std::filesystem::path& inputPath)
{
    auto path = inputPath;

    if (!inputPath.is_absolute())
    {
        path = ass::inputPathAbsolute / inputPath;
    }

    auto fileExists = std::filesystem::exists(path);
    if (!fileExists)
    {
        throw std::runtime_error(std::format("The provided filepath does not exist: {}", path.string()));
    }

    std::ifstream inputStream{path, std::ios_base::binary};

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
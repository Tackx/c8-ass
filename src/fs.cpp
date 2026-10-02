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
#include <utility>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include "fs.h"

namespace ass
{

bool isFileInput()
{
    return !isatty(fileno(stdin));
}

std::string loadFileContentString(const std::filesystem::path& inputPath)
{
    if (inputPath != "")
    {
        ass::filename = inputPath.filename().string();
        size_t size = std::filesystem::file_size(inputPath);

        ass::inputPathAbsolute = std::filesystem::absolute(inputPath).remove_filename();

        std::string content(size, '\0');

        std::ifstream t(inputPath, std::ios::binary);
        if (!t)
        {
            throw std::system_error(errno, std::system_category(), std::format("Failed to open file {}", ass::filename));
        }

        t.read(&content[0], static_cast<long long>(size));

        return content;
    }

    if (isFileInput())
    {
        std::stringstream buffer;
        ass::filename = "[STDIN]";
        ass::inputPathAbsolute = std::filesystem::current_path();

        // Try to load input from stdin
        buffer << std::cin.rdbuf();

        return std::move(buffer).str();
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

    // TODO: Probably can be removed?
    // inputStream.close();

    return bytes;
}

void ensureFilepathExists(std::filesystem::path filePath)
{
    filePath.remove_filename();

    if (filePath.string().size() > 0)
    {
        std::filesystem::create_directories(filePath);
    }
}

std::filesystem::path getAbsoluteFilepath(const std::filesystem::path& inputPath)
{
    auto out = inputPath;

    if (!out.is_absolute())
    {
        out = ass::inputPathAbsolute / inputPath;
    }

    return out;
}

size_t getFileSize(const std::filesystem::path& inputPath)
{
    auto path = getAbsoluteFilepath(inputPath);

    return std::filesystem::file_size(path);
}

void validateIncbinFile(const std::filesystem::path& inputPath)
{
    auto path = getAbsoluteFilepath(inputPath);

    auto exists = std::filesystem::exists(path);
    if (!exists)
    {
        throw std::runtime_error(std::format("The provided filepath does not exist: {}", path.string()));
    }

    auto size = getFileSize(path);
    if (size > 15)
    {
        throw std::runtime_error(std::format("The provided file {} is too large (> 15 bytes): {}", path.string(), size));
    }
}

} // namespace ass
#pragma once

#include <stdexcept>
#include <string>

namespace ass
{

inline std::string filename{};

class NoInputException : public std::runtime_error
{
  public:
    NoInputException(const char* msg) : std::runtime_error(msg)
    {
    }
};

std::string loadFileContent(const std::string& inputPath);

void ensureFilepathExists(std::string& filepath);

} // namespace ass
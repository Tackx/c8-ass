#pragma once

#include <stdexcept>
#include <string>

namespace ass
{

class NoInputException : public std::runtime_error
{
  public:
    NoInputException(const char* msg) : std::runtime_error(msg)
    {
    }
};

std::string loadFileContent(const std::string& inputPath);

} // namespace ass
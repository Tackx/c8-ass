#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

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

std::string loadFileContentString(const std::string& inputPath);
std::vector<uint8_t> loadFileContentBytes(const std::string& inputPath);

void ensureFilepathExists(std::string& filepath);

} // namespace ass
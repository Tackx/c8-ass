#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace ass
{

inline std::string filename{};
inline std::filesystem::path inputPathAbsolute{};

class NoInputException : public std::runtime_error
{
  public:
    NoInputException(const char* msg) : std::runtime_error(msg)
    {
    }
};

bool isFileInput();
std::string loadFileContentString(const std::filesystem::path& inputPath);
std::vector<uint8_t> loadFileContentBytes(const std::filesystem::path& inputPath);
void ensureFilepathExists(std::string& filepath);

} // namespace ass
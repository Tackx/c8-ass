#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ass
{
struct Label
{
    uint16_t addr;
    size_t line;
};

// TODO: Maybe refactor into a class along with the memory location counter and error list
inline std::unordered_map<std::string, Label> labelMemoryMap;

int parseLabels(char** args);
} // namespace ass
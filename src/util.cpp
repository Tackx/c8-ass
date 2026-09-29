#include <algorithm>
#include <cctype>
#include <chrono>
#include <string_view>

#include "logger.h"
#include "util.h"


namespace ass
{

Timer::Timer() : m_start{std::chrono::system_clock::now()} {};

Timer Timer::Start()
{
    return Timer{};
};

Timer::~Timer()
{
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start);

    Logger::Info("Finished in {}", diff);
};

bool equalsIgnoreCase(std::string_view s1, std::string_view s2)
{
    return std::ranges::equal(s1, s2, [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
}

} // namespace ass
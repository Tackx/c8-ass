#include <algorithm>
#include <cctype>
#include <chrono>
#include <ratio>
#include <string_view>

#include "logger.h"
#include "util.h"

namespace ass
{

Timer::Timer() : m_start{std::chrono::steady_clock::now()}
{
}

Timer Timer::Start()
{
    return Timer{};
}

Timer::~Timer()
{
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> diff = now - m_start;

    Logger::Info("Finished in {:.3f} ms", diff.count());
}

bool equalsIgnoreCase(std::string_view s1, std::string_view s2)
{
    return std::ranges::equal(s1, s2, [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
}

} // namespace ass
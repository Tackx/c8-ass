#pragma once

#include <chrono>
#include <string_view>

namespace ass
{

class Timer
{
  private:
    Timer();

    const std::chrono::time_point<std::chrono::steady_clock> m_start;

  public:
    static Timer Start();

    ~Timer();
};

bool equalsIgnoreCase(std::string_view s1, std::string_view s2);

} // namespace ass
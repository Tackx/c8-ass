#include <format>
#include <print>

#include "logger.h"

namespace ass
{

template <typename... Args> void Logger::Info(std::format_string<Args...> format, Args&&... args)
{
    std::println("[ INFO ]: {}", std::format(format, std::forward<Args>(args)...));
}

template <typename... Args> void Logger::Warn(std::format_string<Args...> format, Args&&... args)
{
    std::println("[ WARN ]: {}", std::format(format, std::forward<Args>(args)...));
}

template <typename... Args> void Logger::Err(std::format_string<Args...> format, Args&&... args)
{
    std::println("[ ERR ]: {}", std::format(format, std::forward<Args>(args)...));
}

} // namespace ass
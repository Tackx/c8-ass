#pragma once

#include <format>
#include <print>

namespace ass
{

class Logger
{
    // TODO: Delete constructors?

  public:
#ifndef NDEBUG
    template <typename... Args> static void Debug(std::format_string<Args...> format, Args&&... args)
    {
        std::println("[ DBG ]: {}", std::format(format, std::forward<Args>(args)...));
    }
#else
    template <typename... Args> static void Debug(std::format_string<Args...> format, Args&&...)
    {
        (void)format;
    }
#endif

    template <typename... Args> static void Info(std::format_string<Args...> format, Args&&... args);

    template <typename... Args> static void Warn(std::format_string<Args...> format, Args&&... args);

    template <typename... Args> static void Err(std::format_string<Args...> format, Args&&... args);
};

} // namespace ass
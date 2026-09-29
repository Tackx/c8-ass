#pragma once

#include <format>
#include <print>

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace ass
{

class Logger
{
    inline static bool isTerminal = isatty(fileno(stdout));
    inline static auto yellow = isTerminal ? "\033[33;1m" : "";
    inline static auto red = isTerminal ? "\033[31;1m" : "";
    inline static auto reset = isTerminal ? "\033[0m" : "";

  public:
    Logger() = delete;

#ifndef NDEBUG
    template <typename... Args> static void Debug(std::format_string<Args...> format, Args&&... args)
    {
        std::println("[ DBG ]: {}", std::format(format, std::forward<Args>(args)...));

        std::println("isFile: {}", isTerminal);
    }
#else
    template <typename... Args> static void Debug(std::format_string<Args...> format, Args&&...)
    {
        (void)format;
    }
#endif

    template <typename... Args> static void Info(std::format_string<Args...> format, Args&&... args)
    {
        std::println("[ INFO ]: {}", std::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args> static void Warn(std::format_string<Args...> format, Args&&... args)
    {
        std::println("{}[ WARN ]:{} {}", yellow, reset, std::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args> static void Err(std::format_string<Args...> format, Args&&... args)
    {
        std::println("{}[ ERR ]:{} {}", red, reset, std::format(format, std::forward<Args>(args)...));
    }
};

} // namespace ass
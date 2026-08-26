#include <cstddef>
#include <format>
#include <fstream>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "ass.h"
#include "emitter.h"
#include "instruction.h"
#include "label.h"

using namespace ass;

std::optional<Instruction> parseLine(std::string_view line, size_t lineNr)
{
    size_t i = 0;

    if (line.empty())
    {
        return {};
    }

    if (line.back() == '\r')
    {
        line.remove_suffix(1);
    }

    while (i < line.length() && (line[i] == ' ' || line[i] == '\t' || line[i] == ','))
    {
        i++;
    }

    if (i == line.length() || line[i] == ';')
    {
        return {};
    }

    size_t start = i;

    while (i < line.length() && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',')
    {
        i++;
    }

    if (line[i - 1] == ':')
    {
        i++;
        if (line.length() <= i || line[i] == ';')
        {
            // The label is on a standalone line, no mnem to parse here
            return {};
        }

        start = i;
        // Labels were parsed in the first pass, continue to the mnemonic
        while (i < line.length() && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',')
        {
            i++;
        }
    }

    auto mnem = std::string_view{line}.substr(start, i - start);

    std::vector<std::string_view> rawOps{};

    while (i < line.length())
    {
        // We may have ended on a space/tab/comma above
        while (i < line.length() && (line[i] == ' ' || line[i] == '\t' || line[i] == ','))
        {
            i++;
        }

        if (i >= line.length())
        {
            std::println("End of line reached while parsing ops");

            break;
        }

        if (line[i] == ';')
        {
            std::println("Found comment, stopping parsing line {}", lineNr);

            break;
        }

        auto opStart = i;
        while (i < line.length() && line[i] != ' ' && line[i] != '\t' && line[i] != ';' && line[i] != ',')
        {
            i++;
        }

        auto op = std::string_view{line}.substr(opStart, i - opStart);

        rawOps.push_back(op);
    }

    auto parsedInstruction = parseInstruction(mnem, rawOps, lineNr);

    std::println("Final hex: {:x}", parsedInstruction.encodedHex);

    return parsedInstruction;
}

int emitCode(char** args)
{
    std::ifstream fi{args[1]};
    if (!fi.is_open())
    {
        std::println("Cannot open input file");

        return 1;
    }

    Emitter emitter{args[2]};

    std::string line;

    for (size_t lineNr = 1; std::getline(fi, line); ++lineNr)
    {
        auto parsedInstruction = parseLine(line, lineNr);

        if (parsedInstruction.has_value())
        {
            emitter.emit(*parsedInstruction);
        }
    }

    return 0;
}

int ass::assemble(int argc, char** args)
{
    static constexpr std::string_view USAGE_MSG = "Usage: ass INPUT OUTPUT";

    // TODO: Enable reading from a redirected stream
    if (argc <= 2)
    {

        if (argc <= 1)
        {
            std::println("Missing input path.");
        }

        std::println("Missing output path.");

        std::println("{}", USAGE_MSG);

        return 1;
    }

    int r = 0;

    // First pass, which only parses and stores labels + their memory addresses
    // for substitution in second pass
    r = parseLabels(args);
    if (r != 0)
    {
        return r;
    }

    // Second pass
    r = emitCode(args);

    return r;
}

#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <print>
#include <string>
#include <string_view>

#include "label.h"
#include "parser.h"

using namespace ::ass;

int ass::parseLabels(char** args)
{
    std::ifstream fi{args[1]};
    if (!fi.is_open())
    {
        std::println("Cannot open input file");

        return 1;
    }

    uint16_t memPointer{0x200};

    std::string line;

    for (size_t lineNr = 1; std::getline(fi, line); ++lineNr)
    {
        size_t i = 0;

        if (line.empty())
        {
            continue;
        }

        if (line.back() == '\r')
        {
            line.pop_back();
        }

        while (i < line.length() && (Parser::isWhitespace(line[i]) || Parser::isArgSeparator(line[i])))
        {
            i++;
        }

        if (i == line.length() || Parser::isComment(line[i]))
        {
            continue;
        }

        size_t start = i;

        // TODO: Add another static helper isLabelEnd
        while (i < line.length() && !Parser::isWhitespace(line[i]) && !Parser::isComment(line[i]) && !Parser::isArgSeparator(line[i]) && line[i] != ':')
        {
            i++;
        }

        auto token = line.substr(start, i - start);

        if (line[i] == ':')
        {
            if (labelMemoryMap.contains(token))
            {
                std::println("Found duplicate label on line {}. Label {} is already defined on line {}.", lineNr,
                             std::string_view{token}.substr(0, token.length() - 1), labelMemoryMap[token].line);

                return 1;
            }

            labelMemoryMap[token] = Label{.addr = memPointer, .line = lineNr};

            std::println("IT'S A LABEL: {}", labelMemoryMap.at(token).addr);

            if (i < line.length())
            {
                i++; // Move cursor forward by one char, as we ended on ':'
            }

            while (i < line.length() && (Parser::isWhitespace(line[i])))
            {
                i++;
            }

            // Continue to avoid incrementing the memory pointer, but only if it's a standalone label (on its own line)
            if (i == line.length() || Parser::isComment(line[i]))
            {
                continue;
            }

            // TODO: It would make sense to do semantic checks here in the first pass too to have feedback and stop the process earlier
        }

        memPointer += 2;
    }

    return 0;
}
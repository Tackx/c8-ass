#include <cstddef>
#include <cstdint>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

#include "label.h"
#include "parser.h"

using namespace ::ass;

int ass::parseLabels(std::string_view textContent)
{

    uint16_t memPointer{0x200};

    // TODO: Rewrite using std::ranges
    size_t lineNr = 1;
    for (auto str : textContent | std::views::split('\n'))
    {
        std::string_view line{str};

        size_t i = 0;

        if (line.empty())
        {
            continue;
        }

        if (line.back() == '\r')
        {
            line.remove_suffix(1);
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

        std::string token{line.substr(start, i - start)};

        if (i < line.length() && line[i] == ':')
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

        lineNr++;
    }

    return 0;
}
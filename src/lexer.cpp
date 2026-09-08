#include <cctype>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "lexer.h"

namespace ass
{

Lexer::Lexer(std::string_view fileContent) : m_text{fileContent}, m_cursor{0}, m_line{1}, m_col{1} {};

bool Lexer::isWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

bool Lexer::isComment(char c)
{
    return c == ';';
}

bool Lexer::isArgSeparator(char c)
{
    return c == ',';
}

std::vector<Token> Lexer::getTokens()
{
    std::vector<Token> out;

    while (m_cursor < m_text.length())
    {
        Token t{
            .line = m_line,
            .col = m_col,
            .text = m_text.substr(m_cursor, 1),
        };

        if (m_text[m_cursor] == '\n')
        {
            t.type = TokenType::Newline;

            m_cursor++;
            m_line++;
            m_col = 1;
        }
        else if (m_text[m_cursor] == '\r')
        {
            m_cursor++;

            continue;
        }
        else if (isWhitespace(m_text[m_cursor]))
        {
            m_cursor++;
            m_col++;

            continue;
        }

        else if (isComment(m_text[m_cursor]))
        {
            while (m_cursor < m_text.length() && m_text[m_cursor] != '\n')
            {
                m_cursor++; // Move the cursor
                m_col++;
            }

            continue;
        }
        else if (isArgSeparator(m_text[m_cursor]))
        {
            t.type = TokenType::Comma;

            m_cursor++;
            m_col++;
        }
        else if (m_text[m_cursor] == ':')
        {
            t.type = TokenType::Colon;

            m_cursor++;
            m_col++;
        }
        else if (m_text[m_cursor] == '[')
        {
            t.type = TokenType::LBracket;

            m_cursor++;
            m_col++;
        }
        else if (m_text[m_cursor] == ']')
        {
            t.type = TokenType::RBracket;

            m_cursor++;
            m_col++;
        }

        else if (std::isdigit(m_text[m_cursor]))
        {
            t.type = TokenType::Number;

            bool isHex = false;

            auto start = m_cursor;

            m_cursor++;
            m_col++;

            if (m_text[start] == '0' && m_cursor + 1 < m_text.length() && m_text[m_cursor] == 'x')
            {
                isHex = true;
                m_cursor++;
                m_col++;
            }

            while (m_cursor < m_text.length() && m_text[m_cursor] != ' ' && m_text[m_cursor] != '\n' && m_text[m_cursor] != '\r' && m_text[m_cursor] != ':' &&
                   m_text[m_cursor] != ',' && m_text[m_cursor] != ';' && m_text[m_cursor] != ']')
            {
                if (isHex)
                {
                    if (!std::isxdigit(m_text[m_cursor]))
                    {
                        throw std::runtime_error("Invalid number");
                    }
                }
                else
                {
                    if (!std::isdigit(m_text[m_cursor]))
                    {
                        throw std::runtime_error("Invalid number");
                    }
                }

                m_cursor++; // Move the cursor
                m_col++;
            }

            t.text = m_text.substr(start, m_cursor - start);
        }

        else // It's an identifier
        {
            t.type = TokenType::Identifier;

            auto start = m_cursor;

            m_cursor++;
            m_col++;

            while (m_cursor < m_text.length() && !isWhitespace(m_text[m_cursor]) && m_text[m_cursor] != '\n' && m_text[m_cursor] != '\r' &&
                   m_text[m_cursor] != ':' && !isArgSeparator(m_text[m_cursor]) && !isComment(m_text[m_cursor]) && m_text[m_cursor] != '[' &&
                   m_text[m_cursor] != ']')
            {
                // TODO: Check for L/R brackets and throw if present?

                m_cursor++; // Move the cursor
                m_col++;
            }

            t.text = m_text.substr(start, m_cursor - start);
        }

        out.push_back(t);
    }

    out.push_back(Token{
        .type = TokenType::End,
        .line = m_line,
        .col = m_col,
    });

    return out;
}

} // namespace ass

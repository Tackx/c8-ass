#include <cstddef>
#include <string_view>
#include <vector>
namespace ass
{

enum class TokenType
{
    Unknown,
    Identifier,
    Number,
    Comma,
    Colon,
    LBracket,
    RBracket,
    Newline,
    End
};

struct Token
{
    TokenType type{TokenType::Unknown};
    size_t line{0};
    size_t col{0};
    std::string_view text{};
};

class Lexer
{
  public:
    Lexer(std::string_view fileContent);

    std::vector<Token> getTokens();

  private:
    std::string_view m_text;
    size_t m_cursor;
    size_t m_line;
    size_t m_col;
};

} // namespace ass
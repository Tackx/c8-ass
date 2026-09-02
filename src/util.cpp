#include <algorithm>
#include <cctype>
#include <string_view>

namespace ass
{
bool equalsIgnoreCase(std::string_view s1, std::string_view s2)
{
    return std::ranges::equal(s1, s2, [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
}
} // namespace ass
#include <string>
#include <unordered_map>

namespace ass
{

struct Result
{
    const std::unordered_map<std::string, std::string> flags;
    const std::string inPath;
};

Result getFlags(int argc, char** argv);

} // namespace ass

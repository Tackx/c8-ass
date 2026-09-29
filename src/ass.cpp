#include <exception>
#include <filesystem>
#include <format>
#include <string>
#include <unordered_map>

#include "ass.h"
#include "emitter.h"
#include "flags.h"
#include "lexer.h"
#include "loader.h"
#include "logger.h"
#include "parser.h"
#include "util.h"

namespace ass
{

static constexpr auto defaultOutPath = "./output.ch8";

std::string getOutPath(std::unordered_map<std::string, std::string> flags)
{
    std::string out{defaultOutPath};

    auto outFlag = flags.find("-o");
    if (outFlag != flags.end())
    {
        out = outFlag->second;
    }

    return out;
}

int assemble(int argc, char** argv)
{
    static constexpr auto USAGE_MSG = "Usage: ass [FLAGS]... INPUT\n-o  Specifies the output path and filename. Default = current directory + 'output.ch8'\n";

    try
    {
        auto timer = Timer::Start();

        auto [flags, inPath] = getFlags(argc, argv);

        std::filesystem::path inputPath{inPath};
        auto fileContent = loadFileContentString(inputPath);

        Lexer lexer{fileContent};
        Parser parser{};

        auto outPath = getOutPath(flags);
        ensureFilepathExists(outPath);

        Emitter emitter{outPath};

        auto tokens = lexer.produceTokens();

        // First pass, which only parses and stores labels + their memory addresses
        // for substitution in second pass
        parser.parseLabels(tokens);

        // Second pass
        auto parsedInstructions = parser.parseInstructions(tokens);

        emitter.emit(parsedInstructions);

        Logger::Info("ROM assembled successfully: {}", outPath);

        Logger::Debug("YOOO IT'S DEBUG FROM THE CLASS!!!!! {}", "aaaaaa");

        return 0;
    }
    catch (const NoInputException& e)
    {
        Logger::Err("Exception caught: {}\n", e.what());
        Logger::Info("{}", USAGE_MSG);

        return 1;
    }
    catch (const std::exception& e)
    {
        Logger::Err("Exception caught: {}", e.what());

        return 1;
    }
}

} // namespace ass

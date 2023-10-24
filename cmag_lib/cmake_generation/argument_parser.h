#pragma once

#include <filesystem>
#include <vector>

namespace fs = std::filesystem; // TODO move to some shared header

class ArgumentParser {
public:
    ArgumentParser(int argc, const char **argv);

    const auto &getSourcePath() const { return sourcePath; }
    const auto &getBuildPath() const { return buildPath; }
    const auto &getGraphvizPath() const { return graphvizPath; }
    const auto &getExtraArgs() const { return extraArgs; }
    auto isValid() const { return valid; }

private:
    const char *parseKeyValueArgument(const char *prefix, int &argIndex, const std::string &currentArg, const char *nextArg);

    bool valid = true;
    std::vector<std::string> extraArgs = {};
    fs::path sourcePath = {};
    fs::path buildPath = {};
    fs::path graphvizPath = {};
};

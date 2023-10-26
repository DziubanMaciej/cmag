#pragma once

#include "cmag_lib/utils/filesystem.h"

#include <vector>

class ArgumentParser {
public:
    ArgumentParser(int argc, const char **argv);

    const auto &getSourcePath() const { return sourcePath; }
    const auto &getBuildPath() const { return buildPath; }
    const auto &getGraphvizPath() const { return graphvizPath; }
    const auto &getExtraArgs() const { return extraArgs; }
    auto isValid() const { return valid; }

    void printHelp() {} // TODO
    std::vector<const char *> constructArgsForCmake() const;

private:
    const char *parseKeyValueArgument(std::string_view prefix, int &argIndex, std::string_view currentArg, const char *nextArg);

    int argc = {};
    const char **argv = {};

    bool valid = true;
    std::vector<std::string> extraArgs = {};
    fs::path sourcePath = {};
    fs::path buildPath = {};
    fs::path graphvizPath = {};
};

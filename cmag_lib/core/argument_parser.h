#pragma once

#include "cmag_lib/utils/filesystem.h"

#include <vector>

class ArgumentParser {
public:
    ArgumentParser(int argc, const char **argv);

    const auto &getExtraArgs() const { return extraArgs; }
    auto isValid() const { return valid; }

    const auto &getProjectName() const { return projectName; }
    const auto &getExtraTargetProperties() const { return extraTargetProperties; }
    auto getJsonDebug() const { return jsonDebug; }

    const auto &getSourcePath() const { return sourcePath; }
    const auto &getBuildPath() const { return buildPath; }
    const auto &getGraphvizPath() const { return graphvizPath; }

    std::vector<std::string> constructArgsForCmake() const; // TODO move this out of this class
    void printHelp();

private:
    const char *parseKeyValueArgument(std::string_view prefix, int &argIndex, std::string_view currentArg, const char *nextArg);

    int cmakeArgsStartIndex = {};
    int argc = {};
    const char **argv = {};

    bool valid = true;
    std::vector<std::string> extraArgs = {};

    // Cmag args
    std::string projectName = {};
    std::string extraTargetProperties = {};
    bool jsonDebug = false;

    // Cmake args
    fs::path sourcePath = {};
    fs::path buildPath = {};
    fs::path graphvizPath = {};
};

#pragma once

#include "cmag_core/utils/filesystem.h"

#include <vector>

class DumperArgumentParser {
public:
    DumperArgumentParser(int argc, const char **argv);

    auto isValid() const { return valid; }

    const auto &getProjectName() const { return projectName; }
    const auto &getExtraTargetProperties() const { return extraTargetProperties; }
    auto getJsonDebug() const { return jsonDebug; }
    auto getLaunchGui() const { return launchGui; }
    auto getShowVersion() const { return showVersion; }
    auto getMakeFindPackageGlobal() const { return makeFindPackageGlobal; }

    const auto &getSourcePath() const { return sourcePath; }
    const auto &getBuildPath() const { return buildPath; }

    std::vector<std::string> constructArgsForCmake() const; // TODO move this out of this class
    void printHelp();

private:
    const char *parseKeyValueArgument(std::string_view prefix, int &argIndex, std::string_view currentArg, const char *nextArg);
    bool skipIrrelevantKeyValueArgument(int &argIndex, std::string_view currentArg);

    int cmakeArgsStartIndex = {};
    int argc = {};
    const char **argv = {};

    bool valid = true;

    // Cmag args
    std::string projectName = {};
    std::string extraTargetProperties = {};
    bool showVersion = false;
    bool jsonDebug = false;
    bool launchGui = false;
    bool makeFindPackageGlobal = false;

    // Cmake args
    fs::path sourcePath = {};
    fs::path buildPath = {};
};

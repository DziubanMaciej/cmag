#pragma once

#include "cmag_core/core/cmag_project.h"
#include "cmag_core/utils/filesystem.h"

#include <string_view>

enum class CmagResult {
    Success,
    FileAccessError,
    SubprocessError,
    JsonParseError,
    ProjectCreationError,
    DerivationError,
};

class CmagDumper {
public:
    CmagDumper(std::string_view projectName, bool generationDebug);
    ~CmagDumper();

    CmagResult generateCmake(const fs::path &sourcePath, std::vector<std::string> cmakeArgs, std::string_view extraTargetProperties);
    CmagResult readCmagProjectFromGeneration(const fs::path &buildPath);
    CmagResult generateCmakeAliasPass(const fs::path &sourcePath, std::vector<std::string> cmakeArgs);
    CmagResult writeProjectToFile(const fs::path &buildPath);
    CmagResult cleanupTemporaryFiles();
    CmagResult launchProjectInGui(const fs::path &buildPath);

protected:
    std::string projectName;
    bool generationDebug;
    CmagProject project = {};
    std::vector<fs::path> temporaryFiles = {};
};

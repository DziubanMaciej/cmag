#pragma once

#include "cmag_core/core/cmag_project.h"
#include "cmag_core/utils/filesystem.h"

#include <string_view>

enum class CmagResult {
    Success,
    FileAccessError,
    SubprocessError,
    JsonParseError,
    XdotParseError,
    ProjectCreationError,
};

class CmagDumper {
public:
    CmagDumper(std::string_view projectName, bool generationDebug);
    ~CmagDumper();

    CmagResult generateCmake(const fs::path &sourcePath, const fs::path &buildPath, std::vector<std::string> cmakeArgs, std::string_view extraTargetProperties);
    CmagResult readCmagProjectFromGeneration(const fs::path &buildPath);
    CmagResult generateGraphPositionsForProject(const fs::path &buildPath, const fs::path &graphvizPath);
    CmagResult writeProjectToFile(const fs::path &buildPath);
    CmagResult cleanupTemporaryFiles();
    CmagResult launchProjectInGui(const fs::path &buildPath);

protected:
    bool generationDebug;
    std::string projectName;
    CmagProject project;

    std::vector<fs::path> temporaryFiles = {};
};

#pragma once

#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/utils/filesystem.h"

#include <string_view>

enum class CmagResult {
    Success,
    FileAccessError,
    SubprocessError,
    JsonParseError,
    XdotParseError,
    ProjectCreationError,
};

class Cmag {
public:
    Cmag(std::string_view projectName) : projectName(projectName) {}

    CmagResult generateCmake(const fs::path &sourcePath, std::vector<std::string> cmakeArgs, std::string_view extraTargetProperties);
    CmagResult readCmagProjectFromGeneration(const fs::path &buildPath);
    CmagResult readCmagProjectFromMerge() { return CmagResult::Success; } // TODO implement
    CmagResult generateGraphPositionsForProject(const fs::path &buildPath, const fs::path &graphvizPath);
    CmagResult writeProjectToFile(const fs::path &buildPath);

private:
    std::string projectName;
    CmagProject project;
};

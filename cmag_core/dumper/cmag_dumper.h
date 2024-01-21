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
    CmagDumper(std::string_view projectName,
               bool generationDebug,
               const fs::path &sourcePath,
               const fs::path &buildPath,
               const std::vector<std::string> &cmakeArgsFromUser,
               const std::string &extraTargetProperties);
    ~CmagDumper();

    CmagResult dump();
    CmagResult writeProjectToFile(const fs::path &buildPath);
    CmagResult cleanupTemporaryFiles();
    CmagResult launchProjectInGui(const fs::path &buildPath);

protected:
    CmagResult cmakeMainPass();
    CmagResult readCmakeAfterMainPass();
    CmagResult cmakeSecondPass();
    CmagResult readCmakeAfterSecondPass();

    const std::string projectName;
    const bool generationDebug;
    const fs::path sourcePath;
    const fs::path buildPath;
    const std::vector<std::string> cmakeArgsFromUser;
    const std::string extraTargetProperties;

    CmagProject project = {};
    std::vector<fs::path> temporaryFiles = {};
};

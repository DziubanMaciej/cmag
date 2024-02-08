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
               bool makeFindPackagesGlobal,
               const fs::path &sourcePath,
               const fs::path &buildPath,
               const std::vector<std::string> &cmakeArgsFromUser,
               const std::string &extraTargetProperties);
    ~CmagDumper();

    CmagResult dump();
    CmagResult writeProjectToFile();
    CmagResult cleanupTemporaryFiles();
    CmagResult launchProjectInGui();

protected:
    CmagResult cmakeMainPass();
    CmagResult readCmagProjectName();
    CmagResult readCmakeAfterMainPass();
    CmagResult cmakeSecondPass();
    CmagResult readCmakeAfterSecondPass();
    void verifyWarnings();

    fs::path addTemporaryFile(std::string_view fileName);
    static CmagResult callSubprocess(const char *binaryNameForLogging, const std::vector<std::string> &args);

    std::string projectName;
    const bool generationDebug;
    const bool makeFindPackagesGlobal;
    const fs::path sourcePath;
    const fs::path buildPath;
    const std::vector<std::string> cmakeArgsFromUser;
    const std::string extraTargetProperties;

    CmagProject project = {};
    std::vector<fs::path> temporaryFiles = {};
};

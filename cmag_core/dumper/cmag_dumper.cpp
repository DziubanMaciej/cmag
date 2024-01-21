#include "cmag_dumper.h"

#include "cmag_core/core/version.h"
#include "cmag_core/parse/cmag_json_parser.h"
#include "cmag_core/parse/cmag_json_writer.h"
#include "cmag_core/shim/cmake_lists_shimmer.h"
#include "cmag_core/utils/error.h"
#include "cmag_core/utils/file_utils.h"
#include "cmag_core/utils/string_utils.h"
#include "cmag_core/utils/subprocess.h"

#include <string_view>

#define RETURN_ERROR(expr)                \
    do {                                  \
        const CmagResult r = (expr);      \
        if ((r) != CmagResult::Success) { \
            return (r);                   \
        }                                 \
    } while (false)

CmagDumper::CmagDumper(std::string_view projectName,
                       bool generationDebug,
                       const fs::path &sourcePath,
                       const fs::path &buildPath,
                       const std::vector<std::string> &cmakeArgsFromUser,
                       const std::string &extraTargetProperties)
    : projectName(projectName),
      generationDebug(generationDebug),
      sourcePath(sourcePath),
      buildPath(buildPath),
      cmakeArgsFromUser(cmakeArgsFromUser),
      extraTargetProperties(extraTargetProperties) {}

CmagDumper::~CmagDumper() {
    if (!generationDebug) {
        cleanupTemporaryFiles();
    }
}

CmagResult CmagDumper::dump() {
    RETURN_ERROR(cmakeMainPass());
    RETURN_ERROR(readCmakeAfterMainPass());
    RETURN_ERROR(cmakeSecondPass());
    RETURN_ERROR(readCmakeAfterSecondPass());
    return CmagResult::Success;
}

CmagResult CmagDumper::cmakeMainPass() {
    // Shim original CMakeLists.txt and insert extra CMake code to query information about the build-system
    // and save it to a file.
    CMakeListsShimmer shimmer{sourcePath};
    const ShimResult shimResult = shimmer.shim();
    switch (shimResult) {
    case ShimResult::Success:
        break;
    case ShimResult::InvalidDirectory:
        LOG_ERROR("failed CMakeLists.txt shimming (invalid source directory).\n");
        return CmagResult::FileAccessError;
    case ShimResult::NoPermission:
        LOG_ERROR("failed CMakeLists.txt shimming (no permission).\n");
        return CmagResult::FileAccessError;
    default:
        UNREACHABLE_CODE;
    }

    // Prepare CMake args
    auto cmakeArgs = cmakeArgsFromUser;
    cmakeArgs.emplace_back("-DCMAG_MAIN_FUNCTION=main");
    cmakeArgs.push_back(std::string{"-DCMAG_PROJECT_NAME="} + projectName);
    cmakeArgs.push_back(std::string{"-DCMAG_VERSION="} + cmagVersion.toString());
    cmakeArgs.push_back(std::string{"-DCMAG_EXTRA_TARGET_PROPERTIES="} + extraTargetProperties);
    cmakeArgs.push_back(std::string{"-DCMAG_JSON_DEBUG="} + std::to_string(generationDebug));

    // Call CMake
    const SubprocessResult result = runSubprocess(cmakeArgs);
    switch (result) {
    case SubprocessResult::Success:
        break;
    case SubprocessResult::CreationFailed:
        LOG_ERROR("running CMake failed.");
        return CmagResult::SubprocessError;
    case SubprocessResult::ProcessKilled:
        LOG_ERROR("CMake has been killed.");
        return CmagResult::SubprocessError;
    case SubprocessResult::ProcessFailed:
        LOG_ERROR("CMake failed.");
        return CmagResult::SubprocessError;
    default:
        UNREACHABLE_CODE;
    }

    return CmagResult::Success;
}

CmagResult CmagDumper::readCmakeAfterMainPass() {
    // Read targets list
    std::vector<fs::path> targetsFiles = {};
    {
        std::string fileName = projectName + ".cmag-targets-list";
        fs::path file = buildPath / fileName;
        auto fileContent = readFile(file);
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return CmagResult::FileAccessError;
        }
        temporaryFiles.push_back(file);
        const ParseResult parseResult = CmagJsonParser::parseTargetsFilesListFile(fileContent.value(), targetsFiles);
        if (parseResult.status != ParseResultStatus::Success) {
            LOG_ERROR("failed to parse ", fileName, ". ", parseResult.errorMessage);
            return CmagResult::JsonParseError;
        }
    }

    // Read globals
    CmagGlobals globals = {};
    {
        std::string fileName = std::string(projectName) + ".cmag-globals";
        fs::path file = buildPath / fileName;
        auto fileContent = readFile(file);
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return CmagResult::FileAccessError;
        }
        temporaryFiles.push_back(file);
        const ParseResult parseResult = CmagJsonParser::parseGlobalsFile(fileContent.value(), globals);
        if (parseResult.status != ParseResultStatus::Success) {
            LOG_ERROR("failed to parse ", fileName, ". ", parseResult.errorMessage);
            return CmagResult::JsonParseError;
        }
    }

    // Read targets
    std::vector<CmagTarget> targets = {};
    {
        for (const fs::path &fileName : targetsFiles) {
            fs::path file = buildPath / fileName;
            auto fileContent = readFile(file);
            if (!fileContent.has_value()) {
                LOG_ERROR("failed to read ", fileName);
                return CmagResult::FileAccessError;
            }
            temporaryFiles.push_back(file);
            const ParseResult parseResult = CmagJsonParser::parseTargetsFile(fileContent.value(), targets);
            if (parseResult.status != ParseResultStatus::Success) {
                LOG_ERROR("failed to parse ", fileName, ". ", parseResult.errorMessage);
                return CmagResult::JsonParseError;
            }
        }
    }

    // Assign values to project
    project.getGlobals() = std::move(globals);
    for (CmagTarget &target : targets) {
        bool addResult = project.addTarget(std::move(target));
        if (!addResult) {
            LOG_ERROR("failed to create project");
            return CmagResult::ProjectCreationError;
        }
    }

    return CmagResult::Success;
}

CmagResult CmagDumper::cmakeSecondPass() {
    // TODO this is copy pasted from generateCmake. Make this more common.
    // Shim original CMakeLists.txt and insert extra CMake code to query information about the build-system
    // and save it to a file.
    CMakeListsShimmer shimmer{sourcePath};
    const ShimResult shimResult = shimmer.shim();
    switch (shimResult) {
    case ShimResult::Success:
        break;
    case ShimResult::InvalidDirectory:
        LOG_ERROR("failed CMakeLists.txt shimming (invalid source directory).\n");
        return CmagResult::FileAccessError;
    case ShimResult::NoPermission:
        LOG_ERROR("failed CMakeLists.txt shimming (no permission).\n");
        return CmagResult::FileAccessError;
    default:
        UNREACHABLE_CODE;
    }

    // Derive extra data
    if (!project.deriveData()) {
        LOG_ERROR("failed to derive extra data\n");
        return CmagResult::DerivationError;
    }

    // Prepare CMake args
    auto cmakeArgs = cmakeArgsFromUser;
    cmakeArgs.emplace_back("-DCMAG_MAIN_FUNCTION=aliases");
    cmakeArgs.push_back(std::string{"-DCMAG_PROJECT_NAME="} + projectName);
    cmakeArgs.push_back(std::string{"-DCMAG_ALIASED_TARGETS="} + joinStringWithChar(project.getUnmatchedDependencies(), ';'));

    // Call CMake
    // TODO this is copy pasted from generateCmake. Make this more common.
    const SubprocessResult result = runSubprocess(cmakeArgs);
    switch (result) {
    case SubprocessResult::Success:
        break;
    case SubprocessResult::CreationFailed:
        LOG_ERROR("running CMake failed.");
        return CmagResult::SubprocessError;
    case SubprocessResult::ProcessKilled:
        LOG_ERROR("CMake has been killed.");
        return CmagResult::SubprocessError;
    case SubprocessResult::ProcessFailed:
        LOG_ERROR("CMake failed.");
        return CmagResult::SubprocessError;
    default:
        UNREACHABLE_CODE;
    }

    return CmagResult::Success;
}

CmagResult CmagDumper::readCmakeAfterSecondPass() {
    return CmagResult::Success;
}

CmagResult CmagDumper::writeProjectToFile(const fs::path &buildPath) {
    std::string fileName = std::string(projectName) + ".cmag-project";
    fs::path filePath = buildPath / fileName;
    std::ofstream outFile{filePath, std::ios::out};
    if (!outFile) {
        LOG_ERROR("failed to open ", fileName);
        return CmagResult::FileAccessError;
    }
    CmagJsonWriter::writeProject(project, outFile);
    if (!outFile) {
        LOG_ERROR("failed to write to ", fileName);
        return CmagResult::FileAccessError;
    }

    LOG_INFO("Successfully written project file to ", filePath.string());
    return CmagResult::Success;
}

CmagResult CmagDumper::cleanupTemporaryFiles() {
    for (const fs::path &file : temporaryFiles) {
        if (fs::exists(file)) {
            fs::remove(file);
        }
    }
    temporaryFiles.clear();
    return CmagResult::Success;
}

CmagResult CmagDumper::launchProjectInGui(const fs::path &buildPath) {
    const fs::path browserBinaryPath = getExeLocation().parent_path() / CMAG_BROWSER_BINARY_NAME;

    std::string fileName = std::string(projectName) + ".cmag-project";
    fs::path projectPath = buildPath / fileName;

    std::vector<std::string> browserArgs = {};
    browserArgs.push_back(browserBinaryPath.string());
    browserArgs.push_back(projectPath.string());
    const SubprocessResult result = runSubprocess(browserArgs);
    switch (result) {
    case SubprocessResult::Success:
        return CmagResult::Success;
    case SubprocessResult::CreationFailed:
        LOG_ERROR("running gui failed.");
        return CmagResult::SubprocessError;
    case SubprocessResult::ProcessKilled:
        LOG_ERROR("gui has been killed.");
        return CmagResult::SubprocessError;
    case SubprocessResult::ProcessFailed:
        LOG_ERROR("gui failed.");
        return CmagResult::SubprocessError;
    default:
        UNREACHABLE_CODE;
    }
}

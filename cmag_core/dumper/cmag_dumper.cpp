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
                       bool makeFindPackagesGlobal,
                       const fs::path &sourcePath,
                       const fs::path &buildPath,
                       const std::vector<std::string> &cmakeArgsFromUser,
                       const std::string &extraTargetProperties)
    : projectName(projectName),
      generationDebug(generationDebug),
      makeFindPackagesGlobal(makeFindPackagesGlobal),
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

    RETURN_ERROR(cmakeMainPass());
    RETURN_ERROR(readCmagProjectName());
    RETURN_ERROR(readCmakeAfterMainPass());
    RETURN_ERROR(cmakeSecondPass());
    RETURN_ERROR(readCmakeAfterSecondPass());
    verifyWarnings();
    return CmagResult::Success;
}

CmagResult CmagDumper::cmakeMainPass() {
    // Append cmag-specific arguments and run CMake
    auto cmakeArgs = cmakeArgsFromUser;
    cmakeArgs.emplace_back("-DCMAG_MAIN_FUNCTION=main");
    cmakeArgs.push_back(std::string{"-DCMAG_PROJECT_NAME="} + projectName);
    cmakeArgs.push_back(std::string{"-DCMAG_VERSION="} + cmagVersion.toString());
    cmakeArgs.push_back(std::string{"-DCMAG_EXTRA_TARGET_PROPERTIES="} + extraTargetProperties);
    cmakeArgs.push_back(std::string{"-DCMAG_JSON_DEBUG="} + std::to_string(generationDebug));
    cmakeArgs.push_back(std::string{"-DCMAKE_FIND_PACKAGE_TARGETS_GLOBAL="} + std::to_string(makeFindPackagesGlobal));
    return callSubprocess("CMake", cmakeArgs);
}

CmagResult CmagDumper::readCmagProjectName() {
    const char *fileName = ".cmag-project-name";
    const fs::path file = addTemporaryFile(fileName);
    const auto fileContent = readFile(file);
    if (!fileContent.has_value()) {
        LOG_ERROR("failed to read ", fileName);
        return CmagResult::FileAccessError;
    }
    if (fileContent.value().empty()) {
        LOG_ERROR("file ", fileName, " is empty");
        return CmagResult::ProjectCreationError;
    }
    this->projectName = fileContent.value();
    return CmagResult::Success;
}

CmagResult CmagDumper::readCmakeAfterMainPass() {
    // Read targets list
    std::vector<fs::path> targetsFiles = {};
    {
        const std::string fileName = projectName + ".cmag-targets-list";
        const fs::path file = addTemporaryFile(fileName);
        const auto fileContent = readFile(file);
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return CmagResult::FileAccessError;
        }
        std::vector<std::string> targetFilesNames = {};
        const ParseResult parseResult = CmagJsonParser::parseTargetsFilesListFile(fileContent.value(), targetFilesNames);
        if (parseResult.status != ParseResultStatus::Success) {
            LOG_ERROR("failed to parse ", fileName, ". ", parseResult.errorMessage);
            return CmagResult::JsonParseError;
        }
        for (const std::string &targetFileName : targetFilesNames) {
            targetsFiles.push_back(addTemporaryFile(targetFileName));
        }
    }

    // Read globals
    CmagGlobals globals = {};
    {
        const std::string fileName = std::string(projectName) + ".cmag-globals";
        const fs::path file = addTemporaryFile(fileName);
        const auto fileContent = readFile(file);
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return CmagResult::FileAccessError;
        }
        const ParseResult parseResult = CmagJsonParser::parseGlobalsFile(fileContent.value(), globals);
        if (parseResult.status != ParseResultStatus::Success) {
            LOG_ERROR("failed to parse ", fileName, ". ", parseResult.errorMessage);
            return CmagResult::JsonParseError;
        }
    }

    // Read targets
    std::vector<CmagTarget> targets = {};
    {
        for (const fs::path &file : targetsFiles) {
            const auto fileContent = readFile(file);
            if (!fileContent.has_value()) {
                LOG_ERROR("failed to read ", file.filename().string());
                return CmagResult::FileAccessError;
            }
            const ParseResult parseResult = CmagJsonParser::parseTargetsFile(fileContent.value(), targets);
            if (parseResult.status != ParseResultStatus::Success) {
                LOG_ERROR("failed to parse ", file.filename().string(), ". ", parseResult.errorMessage);
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
    // Derive extra data
    if (!project.deriveData()) {
        LOG_ERROR("failed to derive extra data\n");
        return CmagResult::DerivationError;
    }

    // Append cmag-specific arguments and run CMake
    auto cmakeArgs = cmakeArgsFromUser;
    cmakeArgs.emplace_back("-DCMAG_MAIN_FUNCTION=aliases");
    cmakeArgs.push_back(std::string{"-DCMAG_PROJECT_NAME="} + projectName);
    cmakeArgs.push_back(std::string{"-DCMAG_ALIASED_TARGETS="} + joinStringWithChar(project.getUnmatchedDependencies(), ';'));
    return callSubprocess("CMake", cmakeArgs);
}

CmagResult CmagDumper::readCmakeAfterSecondPass() {
    // Read aliases file
    std::vector<std::pair<std::string, std::string>> aliases = {};
    {
        const std::string fileName = projectName + ".cmag-aliases";
        const fs::path file = addTemporaryFile(fileName);
        const auto fileContent = readFile(file);
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return CmagResult::FileAccessError;
        }
        temporaryFiles.push_back(file);
        const ParseResult parseResult = CmagJsonParser::parseAliasesFile(fileContent.value(), aliases);
        if (parseResult.status != ParseResultStatus::Success) {
            LOG_ERROR("failed to parse ", fileName, ". ", parseResult.errorMessage);
            return CmagResult::JsonParseError;
        }
    }

    for (const auto &alias : aliases) {
        const bool aliasedTargetFound = project.addTargetAlias(alias.first, alias.second);
        if (!aliasedTargetFound) {
            LOG_WARNING("alias \"%s\" for target \"%s\" is found, but the target doesn't exist.");
        }
    }

    return CmagResult::Success;
}

CmagResult CmagDumper::writeProjectToFile() {
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

CmagResult CmagDumper::launchProjectInGui() {
    const fs::path browserBinaryPath = getExeLocation().parent_path() / CMAG_BROWSER_BINARY_NAME;

    std::string fileName = std::string(projectName) + ".cmag-project";
    fs::path projectPath = buildPath / fileName;

    std::vector<std::string> browserArgs = {};
    browserArgs.push_back(browserBinaryPath.string());
    browserArgs.push_back(projectPath.string());
    return callSubprocess("Gui", browserArgs);
}

void CmagDumper::verifyWarnings() {
    if (makeFindPackagesGlobal) {
        const std::string &cmakeVersion = project.getGlobals().cmakeVersion;
        const char *minimumVersion = "3.24.0";
        if (compareCmakeVersions(cmakeVersion.c_str(), minimumVersion) > 0) {
            LOG_WARNING("CMAKE_FIND_PACKAGE_TARGETS_GLOBAL is set to true, but your CMake version is too old. This option is supported starting with ", minimumVersion, ".");
        }
    }
}

fs::path CmagDumper::addTemporaryFile(std::string_view fileName) {
    fs::path file = buildPath / fileName;
    temporaryFiles.push_back(file);
    return file;
}

CmagResult CmagDumper::callSubprocess(const char *binaryNameForLogging, const std::vector<std::string> &args) {
    const SubprocessResult result = runSubprocess(args);
    if (result == SubprocessResult::Success) {
        return CmagResult::Success;
    } else {
        LOG_ERROR(subprocessResultToString(result, binaryNameForLogging));
        return CmagResult::SubprocessError;
    }
}

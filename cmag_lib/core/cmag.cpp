#include "cmag.h"

#include "cmag_lib/parse/cmag_json_parser.h"
#include "cmag_lib/parse/cmag_json_writer.h"
#include "cmag_lib/parse/xdot_parser.h"
#include "cmag_lib/shim/cmake_lists_shimmer.h"
#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/file_utils.h"
#include "cmag_lib/utils/subprocess.h"

#include <string_view>

CmagResult Cmag::generateCmake(const fs::path &sourcePath, std::vector<std::string> cmakeArgs, std::string_view extraTargetProperties) {
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
    cmakeArgs.push_back(std::string{"-DCMAG_PROJECT_NAME="} + projectName);
    cmakeArgs.push_back(std::string{"-DCMAG_EXTRA_TARGET_PROPERTIES="} + extraTargetProperties.data()); // this could be bad if string_view doesn't end with \0

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

CmagResult Cmag::readCmagProjectFromGeneration(const fs::path &buildPath) {
    // Read targets list
    std::vector<fs::path> targetsFiles = {};
    {
        std::string fileName = projectName + ".cmag-targets-list";
        auto fileContent = readFile(buildPath / fileName.c_str());
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return CmagResult::FileAccessError;
        }

        ParseResult parseResult = CmagJsonParser::parseTargetsFilesListFile(fileContent.value(), targetsFiles);
        if (parseResult != ParseResult::Success) {
            LOG_ERROR("failed to parse ", fileName);
            return CmagResult::JsonParseError;
        }
    }

    // Read globals
    CmagGlobals globals = {};
    {
        std::string fileName = std::string(projectName) + ".cmag-globals";
        auto fileContent = readFile(buildPath / fileName.c_str());
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return CmagResult::FileAccessError;
        }
        ParseResult parseResult = CmagJsonParser::parseGlobalsFile(fileContent.value(), globals);
        if (parseResult != ParseResult::Success) {
            LOG_ERROR("failed to parse ", fileName);
            return CmagResult::JsonParseError;
        }
    }

    // Read targets
    std::vector<CmagTarget> targets = {};
    {
        for (const fs::path &fileName : targetsFiles) {
            auto fileContent = readFile(buildPath / fileName);
            if (!fileContent.has_value()) {
                LOG_ERROR("failed to read ", fileName);
                return CmagResult::FileAccessError;
            }
            ParseResult parseResult = CmagJsonParser::parseTargetsFile(fileContent.value(), targets);
            if (parseResult != ParseResult::Success) {
                LOG_ERROR("failed to parse ", fileName);
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
CmagResult Cmag::generateGraphPositionsForProject(const fs::path &buildPath, const fs::path &graphvizPath) {
    // Perform .dot->.xdot conversion
    const std::string xdotPath = buildPath / (projectName + ".xdot");
    {
        const std::string graphvizPathString = graphvizPath.string();
        std::vector<std::string> dotArgs{
            "dot",
            "-Txdot",
            graphvizPathString,
            "-o",
            xdotPath,
        };
        SubprocessResult result = runSubprocess(dotArgs);
        if (result != SubprocessResult::Success) {
            LOG_ERROR("failed to generate xdot");
            return CmagResult::SubprocessError;
        }
    }

    // Parse .xdot
    XdotData xdotData = {};
    {
        const auto xdotContent = readFile(xdotPath);
        if (!xdotContent.has_value()) {
            LOG_ERROR("failed to read ", xdotPath);
            return CmagResult::FileAccessError;
        }
        XdotParseResult parseResult = XdotParser::parse(xdotContent.value(), xdotData);
        if (parseResult != XdotParseResult::Success) {
            LOG_ERROR("failed to parse ", xdotPath);
            return CmagResult::XdotParseError;
        }
    }

    // Assign values to project
    {
        for (const XdotData::Target &target : xdotData.targets) {
            project.addTargetGraphical(target.name, target.x, target.y);
        }
    }

    return CmagResult::Success;
}

CmagResult Cmag::writeProjectToFile(const fs::path &buildPath) {
    std::string fileName = std::string(projectName) + ".cmag-project";
    std::ofstream outFile{buildPath / fileName, std::ios::out};
    if (!outFile) {
        LOG_ERROR("failed to open ", fileName);
        return CmagResult::FileAccessError;
    }
    CmagJsonWriter::writeProject(project, outFile);
    if (!outFile) {
        LOG_ERROR("failed to write to ", fileName);
        return CmagResult::FileAccessError;
    }

    return CmagResult::Success;
}

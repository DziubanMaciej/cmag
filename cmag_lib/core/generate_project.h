#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/json/cmag_json_parser.h"
#include "cmag_lib/json/cmag_json_writer.h"
#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/filesystem.h"
#include "cmag_lib/xdot_parser.h"

#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>

std::optional<std::string> readFile(const fs::path &path) {
    std::ifstream stream(path);
    if (!stream) {
        return {};
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

int readTargetFilesList(const fs::path &buildDir, std::string_view projectName, std::vector<fs::path> &outFiles) {
    std::string fileName = std::string(projectName) + ".cmag-targets-list";
    auto fileContent = readFile(buildDir / fileName.c_str());
    if (!fileContent.has_value()) {
        LOG_ERROR("failed to read ", fileName);
        return 1;
    }

    ParseResult parseResult = CmagJsonParser::parseTargetsFilesListFile(fileContent.value().c_str(), outFiles);
    if (parseResult != ParseResult::Success) {
        LOG_ERROR("failed to parse ", fileName);
        return 1;
    }

    return 0;
}

int readGlobals(const fs::path &buildDir, std::string_view projectName, CmagGlobals &outGlobals) {
    std::string fileName = std::string(projectName) + ".cmag-globals";
    auto fileContent = readFile(buildDir / fileName.c_str());
    if (!fileContent.has_value()) {
        LOG_ERROR("failed to read ", fileName);
        return 1;
    }
    ParseResult parseResult = CmagJsonParser::parseGlobalsFile(fileContent.value().c_str(), outGlobals);
    if (parseResult != ParseResult::Success) {
        LOG_ERROR("failed to parse ", fileName);
        return 1;
    }

    return 0;
}

int readTargets(const fs::path &buildDir, const std::vector<fs::path> &files, std::vector<CmagTarget> &outTargets) {
    for (const fs::path &fileName : files) {
        auto fileContent = readFile(buildDir / fileName);
        if (!fileContent.has_value()) {
            LOG_ERROR("failed to read ", fileName);
            return 1;
        }
        ParseResult parseResult = CmagJsonParser::parseTargetsFile(fileContent.value().c_str(), outTargets);
        if (parseResult != ParseResult::Success) {
            LOG_ERROR("failed to parse ", fileName);
            return 1;
        }
    }

    return 0;
}

int writeProject(const fs::path &buildDir, std::string_view projectName, const CmagProject &project) {
    std::string fileName = std::string(projectName) + ".cmag-project";
    std::ofstream outFile{buildDir / fileName, std::ios::out};
    if (!outFile) {
        LOG_ERROR("failed to open ", fileName);
        return 1;
    }
    CmagJsonWriter::writeProject(project, outFile);
    if (!outFile) {
        LOG_ERROR("failed to write to ", fileName);
        return 1;
    }

    return 0;
}

int createProject(CmagGlobals &&globals, std::vector<CmagTarget> &&targets, CmagProject &outProject) {
    outProject.getGlobals() = std::move(globals);
    for (CmagTarget &target : targets) {
        bool addResult = outProject.addTarget(std::move(target));
        if (!addResult) {
            LOG_ERROR("failed to create project");
            return 1;
        }
    }

    return 0;
}

int generateXdot(const fs::path &buildDir, std::string_view projectName, const fs::path &graphvizPath) {
    const std::string graphvizPathString = graphvizPath.string();
    const std::string xdotPath = buildDir / (std::string{projectName} + ".xdot");
    const char *argv[] = {
        "dot",
        "-Txdot",
        graphvizPathString.c_str(),
        "-o",
        xdotPath.c_str(),
    };
    SubprocessResult result = runSubprocess(sizeof(argv) / sizeof(char*), argv);
    if (result != SubprocessResult::Success) {
        LOG_ERROR("failed to generate xdot");
        return 1;
    }

    const auto xdotContent = readFile(xdotPath);
    if (!xdotContent.has_value()) {
        LOG_ERROR("failed to read ", xdotPath);
        return 1;
    }
    XdotData data = {};
    XdotParseResult parseResult = XdotParser::parse(xdotContent.value(), data);
    if (parseResult != XdotParseResult::Success) {
        LOG_ERROR("failed to parse ", xdotPath);
        return 1;
    }

    return 0;
}

int generateProject(const fs::path &buildDir, std::string_view projectName, const fs::path &graphvizPath) {
    std::vector<fs::path> targetsFiles = {};
    if (int result = readTargetFilesList(buildDir, projectName, targetsFiles); result) {
        return result;
    }

    CmagGlobals globals = {};
    if (int result = readGlobals(buildDir, projectName, globals); result) {
        return result;
    }

    std::vector<CmagTarget> targets = {};
    if (int result = readTargets(buildDir, targetsFiles, targets); result) {
        return result;
    }

    CmagProject project = {};
    if (int result = createProject(std::move(globals), std::move(targets), project); result) {
        return result;
    }

    if (int result = writeProject(buildDir, projectName, project); result) {
        return result;
    }

    if (int result = generateXdot(buildDir, projectName, graphvizPath); result) {
        return result;
    }

    return 0;
}

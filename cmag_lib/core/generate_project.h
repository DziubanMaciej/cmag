#include "cmag_lib/core/cmag_json_parser.h"
#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/filesystem.h"

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

int readConfigs(const fs::path &buildDir, std::string_view projectName, std::vector<std::string> &outConfigs) {
    std::string fileName = std::string(projectName) + ".cmag-configs";
    auto fileContent = readFile(buildDir / fileName.c_str());
    if (!fileContent.has_value()) {
        LOG_ERROR("failed to read ", fileName);
        return 1;
    }
    ParseResult parseResult = CmagJsonParser::parseConfigListFile(fileContent.value().c_str(), outConfigs);
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

int readTargets(const fs::path &buildDir, std::string_view projectName, const std::vector<std::string> &configs, std::vector<CmagTarget> &outTargets) {
    for (const std::string &config : configs) {
        // TODO instead of generating the names, just store them in configs file (change its name).
        std::string fileName = std::string(projectName) + "_" + config + ".cmag-targets";
        auto fileContent = readFile(buildDir / fileName.c_str());
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

int generateProject(const fs::path &buildDir, std::string_view projectName) {
    std::vector<std::string> configs = {};
    if (int result = readConfigs(buildDir, projectName, configs); result) {
        return result;
    }

    CmagGlobals globals = {};
    if (int result = readGlobals(buildDir, projectName, globals); result) {
        return result;
    }

    std::vector<CmagTarget> targets = {};
    if (int result = readTargets(buildDir, projectName, configs, targets); result) {
        return result;
    }

    CmagProject project = {};
    project.getGlobals() = globals;
    for (CmagTarget &target : targets) {
        project.addTarget(std::move(target));
    }

    return 0;
}

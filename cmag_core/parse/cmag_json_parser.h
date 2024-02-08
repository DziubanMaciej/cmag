#pragma once

#include "cmag_core/core/cmag_project.h"
#include "cmag_core/utils/filesystem.h"

#include <nlohmann/json.hpp>
#include <string>

enum class ParseResultStatus {
    Success,
    Malformed,
    InvalidNodeType,
    InvalidValue,
    MissingField,
    DataDerivationFailed,
    VersionMismatch,
};

struct ParseResult {
    ParseResult(ParseResultStatus status, const std::string &errorMessage);

    ParseResultStatus status = ParseResultStatus::Success;
    std::string errorMessage = {};

    const static ParseResult success;
};

class CmagJsonParser {
public:
    static ParseResult parseProject(std::string_view json, CmagProject &outProject);

    static ParseResult parseTargetsFilesListFile(std::string_view json, std::vector<std::string> &outFileNames);
    static ParseResult parseGlobalsFile(std::string_view json, CmagGlobals &outGlobals);
    static ParseResult parseTargetsFile(std::string_view json, std::vector<CmagTarget> &outTargets);
    static ParseResult parseAliasesFile(std::string_view json, std::vector<std::pair<std::string, std::string>> &outAliases);

private:
    static ParseResult validateVersion(const nlohmann::json &node);

    static ParseResult parseGlobalValues(const nlohmann::json &node, CmagGlobals &outGlobals);
    static ParseResult parseGlobalValuesBrowser(const nlohmann::json &node, CmagGlobals::BrowserData &outBrowser);
    static ParseResult parseGlobalValueListDirs(const nlohmann::json &node, CmagGlobals &outGlobals);

    static ParseResult parseTargets(const nlohmann::json &node, std::vector<CmagTarget> &outTargets, bool isProjectFile);
    static ParseResult parseTarget(const nlohmann::json &node, CmagTarget &outTarget, bool isProjectFile);

    static ParseResult parseConfigs(const nlohmann::json &node, CmagTarget &outTarget, bool isProjectFile);
    static ParseResult parseConfigInProjectFile(const nlohmann::json &node, CmagTargetConfig &outConfig);
    static ParseResult parseConfigInTargetsFile(const nlohmann::json &node, CmagTargetConfig &outConfig, const char *targetName);

    static ParseResult parseTargetGraphical(const nlohmann::json &node, CmagTargetGraphicalData &outGraphical);

    static ParseResult parseTargetAliases(const nlohmann::json &node, CmagTarget &outTarget);

    template <typename DstT>
    static ParseResult parseObjectField(const nlohmann::json &node, const char *name, DstT &dst);
};

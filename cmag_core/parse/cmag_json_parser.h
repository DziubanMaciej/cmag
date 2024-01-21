#pragma once

#include "cmag_core/utils/filesystem.h"

#include <nlohmann/json.hpp>
#include <string>

class CmagProject;
struct CmagTarget;
struct CmagGlobals;
struct CmagTargetConfig;
struct CmagTargetGraphicalData;

enum class ParseResultStatus {
    Success,
    Malformed,
    InvalidNodeType,
    InvalidValue,
    MissingField,
    DataDerivationFailed,
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

    static ParseResult parseTargetsFilesListFile(std::string_view json, std::vector<fs::path> &outFiles);
    static ParseResult parseGlobalsFile(std::string_view json, CmagGlobals &outGlobals);
    static ParseResult parseTargetsFile(std::string_view json, std::vector<CmagTarget> &outTargets);
    static ParseResult parseAliasesFile(std::string_view json, std::vector<std::pair<std::string, std::string>> &outAliases);

private:
    static ParseResult parseGlobalValues(const nlohmann::json &node, CmagGlobals &outGlobals);
    static ParseResult parseGlobalValueListDirs(const nlohmann::json &node, CmagGlobals &outGlobals);

    static ParseResult parseTargets(const nlohmann::json &node, std::vector<CmagTarget> &outTargets, bool isProjectFile);
    static ParseResult parseTarget(const nlohmann::json &node, CmagTarget &outTarget, bool isProjectFile);

    static ParseResult parseConfigs(const nlohmann::json &node, CmagTarget &outTarget, bool isProjectFile);
    static ParseResult parseConfigInProjectFile(const nlohmann::json &node, CmagTargetConfig &outConfig);
    static ParseResult parseConfigInTargetsFile(const nlohmann::json &node, CmagTargetConfig &outConfig, const char *targetName);

    static ParseResult parseTargetGraphical(const nlohmann::json &node, CmagTargetGraphicalData &outGraphical);

    template <typename DstT>
    static ParseResult parseObjectField(const nlohmann::json &node, const char *name, DstT &dst);
};

#pragma once

#include "cmag_lib/utils/filesystem.h"

#include <nlohmann/json.hpp>
#include <string>

class CmagProject;
struct CmagTarget;
struct CmagGlobals;

enum class ParseResult {
    Success,
    Malformed,
    InvalidNodeType,
    InvalidValue,
    MissingField,
};

class CmagJsonParser {
public:
    static ParseResult parseProject(const char *json, CmagProject &outProject);

    static ParseResult parseTargetsFilesListFile(const char *json, std::vector<fs::path> &outFiles);
    static ParseResult parseGlobalsFile(const char *json, CmagGlobals &outGlobals);
    static ParseResult parseTargetsFile(const char *json, std::vector<CmagTarget> &outTargets);

private:
    static void parseGlobalValues(const nlohmann::json &node, CmagGlobals &outGlobals);

    static ParseResult parseTargets(const nlohmann::json &node, std::vector<CmagTarget> &outTargets);
    static ParseResult parseTarget(const nlohmann::json &node, CmagTarget &outTarget);

    static ParseResult parseConfigs(const nlohmann::json &node, CmagTarget &outTarget);
    static ParseResult parseConfig(const nlohmann::json &node, std::string_view configName, CmagTarget &outTarget);

    static ParseResult parseTargetProperties(const nlohmann::json &node, const std::string &config, CmagTarget &outTarget);

    template <typename DstT>
    static ParseResult parseObjectField(const nlohmann::json &node, const char *name, DstT &dst);
};

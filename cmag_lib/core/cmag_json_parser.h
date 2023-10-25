#pragma once

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

    static ParseResult parseConfigListFile(const char *json, std::vector<std::string> &outConfigs);
    static ParseResult parseGlobalsFile(const char *json, CmagGlobals &outGlobals);
    static ParseResult parseTargetsFile(const char *json, CmagProject &outProject);

private:
    static void parseGlobalValues(const nlohmann::json &node, CmagGlobals &outGlobals);
    static ParseResult parseTargets(const nlohmann::json &node, const std::string &config, CmagProject &outProject);
    static ParseResult parseTarget(const nlohmann::json &node, const std::string &config, CmagTarget &outTarget);
    static ParseResult parseTargetProperties(const nlohmann::json &node, const std::string &config, CmagTarget &outTarget);

    template <typename DstT>
    static ParseResult parseObjectField(const nlohmann::json &node, const char *name, DstT &dst);
};

#pragma once
#include <nlohmann/json.hpp>
#include <string>

class CmagProject;
struct CmagTarget;

enum class ParseResult {
    Success,
    Malformed,
    InvalidNodeType,
    InvalidValue,
    MissingField,
};

class CmagProjectParser {
public:
    static ParseResult parseProject(const char *json, CmagProject &outProject);

private:
    static void parseGlobalValues(const nlohmann::json &node, CmagProject &outProject);
    static ParseResult parseTargets(const nlohmann::json &node, CmagProject &outProject);
    static ParseResult parseTarget(const nlohmann::json &node, CmagProject &outProject);
    template<typename DstT>
    static ParseResult parseObjectField(const nlohmann::json &node, const char *name, DstT &dst);
    static ParseResult parseTargetProperties(const nlohmann::json &node, CmagTarget &outTarget);
};

#include "cmag_json_parser.h"

#include "cmag_core/core/cmag_project.h"
#include "cmag_core/parse/enum_serialization.h"
#include "cmag_core/utils/error.h"

#define RETURN_ERROR(expr)                              \
    do {                                                \
        const ParseResult r = (expr);                   \
        if ((r.status) != ParseResultStatus::Success) { \
            return (r);                                 \
        }                                               \
    } while (false)

ParseResult::ParseResult(ParseResultStatus status, const std::string &errorMessage)
    : status(status),
      errorMessage(errorMessage) {
    const bool isSuccess = status == ParseResultStatus::Success;
    FATAL_ERROR_IF(isSuccess != errorMessage.empty(), "Invalid parse result");
}

const ParseResult ParseResult::success(ParseResultStatus::Success, "");

ParseResult CmagJsonParser::parseTargetsFilesListFile(std::string_view json, std::vector<fs::path> &outFiles) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return {ParseResultStatus::Malformed, "File is malformed"};
    }

    if (!node.is_array()) {
        return {ParseResultStatus::InvalidNodeType, "Root node should be an array"};
    }

    for (const nlohmann::json &configNode : node) {
        if (!configNode.is_string()) {
            return {ParseResultStatus::InvalidNodeType, "Target should be a string value"};
        }
        outFiles.push_back(configNode.get<fs::path>());
    }

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseGlobalsFile(std::string_view json, CmagGlobals &outGlobals) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return {ParseResultStatus::Malformed, "File is malformed"};
    }

    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Root node should be an object"};
    }

    RETURN_ERROR(parseGlobalValues(node, outGlobals));

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseTargetsFile(std::string_view json, std::vector<CmagTarget> &outTargets) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return {ParseResultStatus::Malformed, "File is malformed"};
    }
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Root node should be an object"};
    }

    return parseTargets(node, outTargets, false);
}

ParseResult CmagJsonParser::parseAliasesFile(std::string_view json, std::vector<std::pair<std::string, std::string>> &outAliases) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return {ParseResultStatus::Malformed, "File is malformed"};
    }
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Root node should be an object"};
    }

    for (auto aliasNodeIt = node.begin(); aliasNodeIt != node.end(); aliasNodeIt++) {
        if (!aliasNodeIt->is_string()) {
            return {ParseResultStatus::InvalidNodeType, "Actual target name should be a string"};
        }
        outAliases.emplace_back(aliasNodeIt.key(), aliasNodeIt.value());
    }

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseProject(std::string_view json, CmagProject &outProject) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return {ParseResultStatus::Malformed, "File is malformed"};
    }

    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Root node should be an object"};
    }

    if (auto globalsNodeIt = node.find("globals"); globalsNodeIt != node.end()) {
        RETURN_ERROR(parseGlobalValues(*globalsNodeIt, outProject.getGlobals()));
    } else {
        return {ParseResultStatus::MissingField, "Missing globals node"};
    }

    if (auto targetsNodeIt = node.find("targets"); targetsNodeIt != node.end()) {
        std::vector<CmagTarget> targets{};
        RETURN_ERROR(parseTargets(*targetsNodeIt, targets, true));
        for (CmagTarget &target : targets) {
            const std::string targetName = target.name;
            bool addResult = outProject.addTarget(std::move(target));
            if (!addResult) {
                return {ParseResultStatus::MissingField, LOG_TO_STRING("Failed to add target ", targetName, " to the project")};
            }
        }
    } else {
        return {ParseResultStatus::MissingField, "Missing targets node"};
    }

    if (!outProject.deriveData()) {
        // TODO return some meaningful string from data derivation
        return {ParseResultStatus::DataDerivationFailed, "Data derivation failed"};
    }
    return ParseResult::success;
}

ParseResult CmagJsonParser::parseGlobalValues(const nlohmann::json &node, CmagGlobals &outGlobals) {
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Globals node should be an object"};
    }

#define PARSE_GLOBAL_FIELD(name) RETURN_ERROR(parseObjectField(node, #name, outGlobals.name))
    PARSE_GLOBAL_FIELD(darkMode);
    PARSE_GLOBAL_FIELD(needsLayout);
    PARSE_GLOBAL_FIELD(selectedConfig);
    PARSE_GLOBAL_FIELD(cmagVersion);
    PARSE_GLOBAL_FIELD(cmakeVersion);
    PARSE_GLOBAL_FIELD(cmakeProjectName);
    PARSE_GLOBAL_FIELD(useFolders);
    PARSE_GLOBAL_FIELD(sourceDir);
    PARSE_GLOBAL_FIELD(buildDir);
    PARSE_GLOBAL_FIELD(generator);
    PARSE_GLOBAL_FIELD(compilerId);
    PARSE_GLOBAL_FIELD(compilerVersion);
    PARSE_GLOBAL_FIELD(os);
    PARSE_GLOBAL_FIELD(cmagProjectName);
#undef PARSE_GLOBAL_FIELD

    if (auto listDirsNodeIt = node.find("listDirs"); listDirsNodeIt != node.end()) {
        RETURN_ERROR(parseGlobalValueListDirs(*listDirsNodeIt, outGlobals));
    } else {
        return {ParseResultStatus::MissingField, "Missing listDirs node"};
    }

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseGlobalValueListDirs(const nlohmann::json &node, CmagGlobals &outGlobals) {
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "List dirs node should be an object"};
    }

    // First pass - gather all files
    for (auto listDirNodeIt = node.begin(); listDirNodeIt != node.end(); listDirNodeIt++) {
        CmagListDir listDir = {};
        listDir.name = listDirNodeIt.key();
        outGlobals.listDirs.push_back(listDir);
    }

    // Second pass - scan for children indices. This has quadratic complexity, but there usually aren't
    // thousands of CMakeLists.txt files, so we should be good.
    size_t i = 0u;
    for (auto listDirNodeIt = node.begin(); listDirNodeIt != node.end(); listDirNodeIt++, i++) {
        CmagListDir &listDir = outGlobals.listDirs[i];

        nlohmann::json childrenNode = listDirNodeIt.value();
        if (!childrenNode.is_array()) {
            return {ParseResultStatus::InvalidNodeType, "List dir's subdirs node should be an array"};
        }

        for (auto &childNodeIt : childrenNode) {
            if (!childNodeIt.is_string()) {
                return {ParseResultStatus::InvalidNodeType, "List dir's subdir should be a string"};
            }

            const std::string childName = childNodeIt.get<std::string>();
            bool foundIndex = false;
            for (size_t j = 0; j < outGlobals.listDirs.size(); j++) {
                if (outGlobals.listDirs[j].name == childName) {
                    listDir.childIndices.push_back(j);
                    foundIndex = true;
                    break;
                }
            }

            if (!foundIndex) {
                return {ParseResultStatus::MissingField, LOG_TO_STRING("Invalid list dir's subdir mentioned: ", childName)};
            }
        }
    }

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseTargets(const nlohmann::json &node, std::vector<CmagTarget> &outTargets, bool isProjectFile) {
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Targets node should be an object"};
    }

    for (auto targetNodeIt = node.begin(); targetNodeIt != node.end(); targetNodeIt++) {
        CmagTarget target{};
        target.name = targetNodeIt.key();
        if (target.name.empty()) {
            return {ParseResultStatus::InvalidValue, "Target name is empty"};
        }

        RETURN_ERROR(parseTarget(*targetNodeIt, target, isProjectFile));
        outTargets.push_back(std::move(target));
    }

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseTarget(const nlohmann::json &node, CmagTarget &outTarget, bool isProjectFile) {
    FATAL_ERROR_IF(outTarget.name.empty(), "Parsing target with empty name");

    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Target node should be an object"};
    }

    RETURN_ERROR(parseObjectField(node, "type", outTarget.type));
    if (outTarget.type == CmagTargetType::Invalid) {
        return {ParseResultStatus::InvalidValue, LOG_TO_STRING("Invalid type specified for target ", outTarget.name)};
    }

    RETURN_ERROR(parseObjectField(node, "isImported", outTarget.isImported));

    if (auto configsNodeIt = node.find("configs"); configsNodeIt != node.end()) {
        RETURN_ERROR(parseConfigs(*configsNodeIt, outTarget, isProjectFile));
    } else {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("Missing configs node for target ", outTarget.name)};
    }

    if (auto configsNodeIt = node.find("graphical"); configsNodeIt != node.end()) {
        RETURN_ERROR(parseTargetGraphical(*configsNodeIt, outTarget.graphical));
    } else if (isProjectFile) {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("Missing target graphical node for target ", outTarget.name)};
    }

    if (auto aliasesNodeIt = node.find("aliases"); aliasesNodeIt != node.end()) {
        RETURN_ERROR(parseTargetAliases(*aliasesNodeIt, outTarget));
    } else {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("Missing aliases node for target ", outTarget.name)};
    }

    RETURN_ERROR(parseObjectField(node, "listDir", outTarget.listDirName));

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseConfigs(const nlohmann::json &node, CmagTarget &outTarget, bool isProjectFile) {
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Configs node should be an object"};
    }

    if (node.empty()) {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("No configs specified for target ", outTarget.name)};
    }

    for (auto configIt = node.begin(); configIt != node.end(); configIt++) {
        CmagTargetConfig &config = outTarget.getOrCreateConfig(configIt.key());
        if (isProjectFile) {
            RETURN_ERROR(parseConfigInProjectFile(*configIt, config));
        } else {
            RETURN_ERROR(parseConfigInTargetsFile(*configIt, config, outTarget.name.c_str()));
        }
    }

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseConfigInProjectFile(const nlohmann::json &node, CmagTargetConfig &outConfig) {
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Config node should be an object"};
    }

    for (auto it = node.begin(); it != node.end(); it++) {
        CmagTargetProperty property = {it.key(), it.value()};
        outConfig.properties.push_back(std::move(property));
    }
    return ParseResult::success;
}

ParseResult CmagJsonParser::parseConfigInTargetsFile(const nlohmann::json &node, CmagTargetConfig &outConfig, const char *targetName) {
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Config node should be an object"};
    }

    if (auto propertiesNodeIt = node.find("non_genexable"); propertiesNodeIt != node.end()) {
        for (auto it = propertiesNodeIt->begin(); it != propertiesNodeIt->end(); it++) {
            CmagTargetProperty property = {it.key(), it.value()};
            outConfig.properties.push_back(std::move(property));
        }
    } else {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("Missing non_genexable field for ", targetName)};
    }

    if (auto propertiesNodeIt = node.find("genexable_evaled"); propertiesNodeIt != node.end()) {
        for (auto it = propertiesNodeIt->begin(); it != propertiesNodeIt->end(); it++) {
            CmagTargetProperty property = {it.key(), it.value()};
            outConfig.properties.push_back(std::move(property));
        }
    } else {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("Missing genexable_evaled field for ", targetName)};
    }

    if (auto propertiesNodeIt = node.find("genexable"); propertiesNodeIt != node.end()) {
        for (auto it = propertiesNodeIt->begin(); it != propertiesNodeIt->end(); it++) {
            std::string propertyValue = it.value();
            outConfig.fixupWithNonEvaled(it.key(), propertyValue);
        }
    } else {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("Missing genexable field for ", targetName)};
    }

    return ParseResult::success;
}

ParseResult CmagJsonParser::parseTargetGraphical(const nlohmann::json &node, CmagTargetGraphicalData &outGraphical) {
    if (!node.is_object()) {
        return {ParseResultStatus::InvalidNodeType, "Target graphical node should be an object"};
    }

    RETURN_ERROR(parseObjectField(node, "x", outGraphical.x));
    RETURN_ERROR(parseObjectField(node, "y", outGraphical.y));
    RETURN_ERROR(parseObjectField(node, "hideConnections", outGraphical.hideConnections));

    return ParseResult::success;
}
ParseResult CmagJsonParser::parseTargetAliases(const nlohmann::json &node, CmagTarget &outTarget) {
    if (!node.is_array()) {
        return {ParseResultStatus::InvalidNodeType, "Target aliases node should be an array"};
    }

    for (const nlohmann::json &aliasNode : node) {
        if (!aliasNode.is_string()) {
            return {ParseResultStatus::InvalidNodeType, "Target alias should be a string value"};
        }

        outTarget.aliases.push_back(aliasNode.get<std::string>());
    }

    return ParseResult::success;
}

template <typename DstT>
ParseResult CmagJsonParser::parseObjectField(const nlohmann::json &node, const char *name, DstT &dst) {
    if (auto it = node.find(name); it != node.end()) {
        dst = it->get<DstT>();
        return ParseResult::success;
    } else {
        return {ParseResultStatus::MissingField, LOG_TO_STRING("Missing ", name, " field")};
    }
}

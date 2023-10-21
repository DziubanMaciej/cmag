#include "cmag_project.h"
#include "cmag_project_parser.h"

#include "cmag_lib/utils/error.h"

NLOHMANN_JSON_SERIALIZE_ENUM(CmagTargetType,
                             {
                                 {CmagTargetType::Invalid, ""},
                                 {CmagTargetType::StaticLibrary, "StaticLibrary"},
                                 {CmagTargetType::ModuleLibrary, "ModuleLibrary"},
                                 {CmagTargetType::SharedLibrary, "SharedLibrary"},
                                 {CmagTargetType::ObjectLibrary, "ObjectLibrary"},
                                 {CmagTargetType::InterfaceLibrary, "InterfaceLibrary"},
                                 {CmagTargetType::Executable, "Executable"},
                             })

ParseResult CmagProjectParser::parseProject(const char *json, CmagProject &outProject) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return ParseResult::Malformed;
    }

    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    parseGlobalValues(node, outProject);

    if (auto it = node.find("targets"); it != node.end()) {
        return parseTargets(*it, outProject);
    } else {
        return ParseResult::MissingField;
    }
}

void CmagProjectParser::parseGlobalValues(const nlohmann::json &node, CmagProject &outProject) {
    FATAL_ERROR_IF(!node.is_object(), "node should be an object"); // This should already be checked, hence assertion.

    if (bool value{}; parseObjectField(node, "darkMode", value) == ParseResult::Success) {
        outProject.setDarkMode(value);
    }
}

ParseResult CmagProjectParser::parseTargets(const nlohmann::json &node, CmagProject &outProject) {
    if (!node.is_array()) {
        return ParseResult::InvalidNodeType;
    }

    for (const nlohmann::json &targetNode : node) {
        ParseResult result = parseTarget(targetNode, outProject);
        if (result != ParseResult::Success) {
            return result;
        }
    }

    return ParseResult::Success;
}

ParseResult CmagProjectParser::parseTarget(const nlohmann::json &node, CmagProject &outProject) {
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    ParseResult result = ParseResult::Success;

    CmagTarget target = {};
    result = parseObjectField(node, "name", target.name);
    if (result != ParseResult::Success) {
        return result;
    }

    result = parseObjectField(node, "type", target.type);
    if (result != ParseResult::Success) {
        return result;
    }
    if (target.type == CmagTargetType::Invalid) {
        return ParseResult::InvalidValue;
    }

    auto propertiesIt = node.find("properties");
    if (propertiesIt == node.end()) {
        return ParseResult::MissingField;
    }
    result = parseTargetProperties(*propertiesIt, target);
    if (result != ParseResult::Success) {
        return result;
    }

    outProject.addTarget(std::move(target));
    return result;
}

template <typename DstT>
ParseResult CmagProjectParser::parseObjectField(const nlohmann::json &node, const char *name, DstT &dst) {
    if (auto it = node.find(name); it != node.end()) {
        dst = it->get<DstT>();
        return ParseResult::Success;
    } else {
        return ParseResult::MissingField;
    }
}

ParseResult CmagProjectParser::parseTargetProperties(const nlohmann::json &node, CmagTarget &outTarget) {
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    for (auto it = node.begin(); it != node.end(); it++) {
        CmagTarget::Property property = {it.key(), it.value()};
        outTarget.properties.push_back(std::move(property));
    }

    return ParseResult::Success;
}

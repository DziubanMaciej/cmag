#include "cmag_json_parser.h"

#include "cmag_lib/core/cmag_project.h"
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

ParseResult CmagJsonParser::parseConfigListFile(const char *json, std::vector<std::string> &outConfigs) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return ParseResult::Malformed;
    }

    if (!node.is_array()) {
        return ParseResult::InvalidNodeType;
    }

    for (const nlohmann::json &configNode : node) {
        if (!configNode.is_string()) {
            return ParseResult::InvalidNodeType;
        }
        outConfigs.push_back(configNode.get<std::string>());
    }

    return ParseResult::Success;
}

ParseResult CmagJsonParser::parseGlobalsFile(const char *json, CmagGlobals &outGlobals) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return ParseResult::Malformed;
    }

    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    parseGlobalValues(node, outGlobals);

    return ParseResult::Success;
}

ParseResult CmagJsonParser::parseTargetsFile(const char *json, CmagProject &outProject) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return ParseResult::Malformed;
    }
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    // Read config of current targets file
    std::string config{};
    ParseResult result = parseObjectField(node, "config", config);
    if (result != ParseResult::Success) {
        return result;
    }

    // Read all targets and their properties for current config
    if (auto targetsNode = node.find("targets"); targetsNode != node.end()) {
        return parseTargets(*targetsNode, config, outProject);
    } else {
        return ParseResult::MissingField;
    }
}

ParseResult CmagJsonParser::parseProject(const char *json, CmagProject &outProject) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return ParseResult::Malformed;
    }

    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    if (auto globalsNode = node.find("globals"); globalsNode != node.end()) {
        parseGlobalValues(*globalsNode, outProject.getGlobals());
    } else {
        return ParseResult::MissingField;
    }


    for (auto targetsNode = node.begin(); targetsNode != node.end(); targetsNode++) {
        const std::string &name = targetsNode.key();
        if (name.find("targets") != std::string::npos && name.size() > strlen("targets")) {
            std::string config = name.c_str() + strlen("targets");
            ParseResult result = parseTargets(*targetsNode, config, outProject);
            if (result != ParseResult::Success) {
                return result;
            }
        }
    }

    return ParseResult::Success;
}

void CmagJsonParser::parseGlobalValues(const nlohmann::json &node, CmagGlobals &outGlobals) {
    FATAL_ERROR_IF(!node.is_object(), "node should be an object"); // This should already be checked, hence assertion.

    parseObjectField(node, "darkMode", outGlobals.darkMode); // optional, ignore result
}

ParseResult CmagJsonParser::parseTargets(const nlohmann::json &node, const std::string &config, CmagProject &outProject) {
    if (!node.is_array()) {
        return ParseResult::InvalidNodeType;
    }

    for (const nlohmann::json &targetNode : node) {
        CmagTarget target{};
        ParseResult result = parseTarget(targetNode, config, target);
        if (result != ParseResult::Success) {
            return result;
        }
        outProject.addTarget(std::move(target));
    }

    return ParseResult::Success;
}

ParseResult CmagJsonParser::parseTarget(const nlohmann::json &node, const std::string &config, CmagTarget &outTarget) {
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    ParseResult result = ParseResult::Success;

    result = parseObjectField(node, "name", outTarget.name);
    if (result != ParseResult::Success) {
        return result;
    }

    result = parseObjectField(node, "type", outTarget.type);
    if (result != ParseResult::Success) {
        return result;
    }
    if (outTarget.type == CmagTargetType::Invalid) {
        return ParseResult::InvalidValue;
    }

    auto propertiesIt = node.find("properties");
    if (propertiesIt == node.end()) {
        return ParseResult::MissingField;
    }
    result = parseTargetProperties(*propertiesIt, config, outTarget);
    if (result != ParseResult::Success) {
        return result;
    }

    return result;
}

ParseResult CmagJsonParser::parseTargetProperties(const nlohmann::json &node, const std::string &config, CmagTarget &outTarget) {
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    // Find properties list for current config
    // TODO: turn this to CmagTarget method?
    auto propertiesIt = std::find_if(outTarget.properties.begin(), outTarget.properties.end(), [&config](const auto &props) {
        return config == props.first;
    });
    CmagTarget::Properties *properties = nullptr;
    if (propertiesIt == outTarget.properties.end()) {
        outTarget.properties.push_back({config, {}});
        properties = &outTarget.properties.back();
    } else {
        properties = &*propertiesIt; // wtf, why is there no get()?
    }

    // Read properties from json and add them to current config's properties.
    for (auto it = node.begin(); it != node.end(); it++) {
        CmagTarget::Property property = {it.key(), it.value()};
        properties->second.push_back(std::move(property));
    }

    return ParseResult::Success;
}

template <typename DstT>
ParseResult CmagJsonParser::parseObjectField(const nlohmann::json &node, const char *name, DstT &dst) {
    if (auto it = node.find(name); it != node.end()) {
        dst = it->get<DstT>();
        return ParseResult::Success;
    } else {
        return ParseResult::MissingField;
    }
}

#include "cmag_json_parser.h"

#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/utils/error.h"

NLOHMANN_JSON_SERIALIZE_ENUM(CmagTargetType,
                             {
                                 {CmagTargetType::Invalid, ""},
                                 {CmagTargetType::StaticLibrary, "STATIC_LIBRARY"},
                                 {CmagTargetType::ModuleLibrary, "MODULE_LIBRARY"},
                                 {CmagTargetType::SharedLibrary, "SHARED_LIBRARY"},
                                 {CmagTargetType::ObjectLibrary, "OBJECT_LIBRARY"},
                                 {CmagTargetType::InterfaceLibrary, "INTERFACE_LIBRARY"},
                                 {CmagTargetType::Executable, "EXECUTABLE"},
                             })

ParseResult CmagJsonParser::parseTargetsFilesListFile(const char *json, std::vector<fs::path> &outFiles) {
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
        outFiles.push_back(configNode.get<fs::path>());
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

ParseResult CmagJsonParser::parseTargetsFile(const char *json, std::vector<CmagTarget> &outTargets) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return ParseResult::Malformed;
    }
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    return parseTargets(node, outTargets);
}

ParseResult CmagJsonParser::parseProject(const char *json, CmagProject &outProject) {
    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    if (node.is_discarded()) {
        return ParseResult::Malformed;
    }

    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    if (auto globalsNodeIt = node.find("globals"); globalsNodeIt != node.end()) {
        parseGlobalValues(*globalsNodeIt, outProject.getGlobals());
    } else {
        return ParseResult::MissingField;
    }

    if (auto targetsNodeIt = node.find("targets"); targetsNodeIt != node.end()) {
        std::vector<CmagTarget> targets{};
        ParseResult result = parseTargets(*targetsNodeIt, targets);
        if (result != ParseResult::Success) {
            return result;
        }
        for (CmagTarget &target : targets) {
            outProject.addTarget(std::move(target));
        }
    } else {
        return ParseResult::MissingField;
    }

    return ParseResult::Success;
}

void CmagJsonParser::parseGlobalValues(const nlohmann::json &node, CmagGlobals &outGlobals) {
    FATAL_ERROR_IF(!node.is_object(), "node should be an object"); // This should already be checked, hence assertion.

    parseObjectField(node, "darkMode", outGlobals.darkMode); // optional, ignore result
}

ParseResult CmagJsonParser::parseTargets(const nlohmann::json &node, std::vector<CmagTarget> &outTargets) {
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    for (auto targetNodeIt = node.begin(); targetNodeIt != node.end(); targetNodeIt++) {
        CmagTarget target{};
        target.name = targetNodeIt.key(); // TODO verify
        ParseResult result = parseTarget(*targetNodeIt, target);
        if (result != ParseResult::Success) {
            return result;
        }

        outTargets.push_back(std::move(target));
    }

    return ParseResult::Success;
}

ParseResult CmagJsonParser::parseTarget(const nlohmann::json &node, CmagTarget &outTarget) {
    FATAL_ERROR_IF(outTarget.name == "", "Parsing target with empty name");

    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    ParseResult result = ParseResult::Success;

    result = parseObjectField(node, "type", outTarget.type);
    if (result != ParseResult::Success) {
        return result;
    }
    if (outTarget.type == CmagTargetType::Invalid) {
        return ParseResult::InvalidValue;
    }

    if (auto configsNodeIt = node.find("configs"); configsNodeIt != node.end()) {
        return parseConfigs(*configsNodeIt, outTarget);
    } else {
        return ParseResult::MissingField;
    }
}

ParseResult CmagJsonParser::parseConfigs(const nlohmann::json &node, CmagTarget &outTarget) {
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    if (node.empty()) {
        return ParseResult::MissingField;
    }

    for (auto configIt = node.begin(); configIt != node.end(); configIt++) {
        ParseResult result = parseConfig(*configIt, configIt.key(), outTarget);
        if (result != ParseResult::Success) {
            return result;
        }
    }

    return ParseResult::Success;
}

ParseResult CmagJsonParser::parseConfig(const nlohmann::json &node, std::string_view configName, CmagTarget &outTarget) {
    if (!node.is_object()) {
        return ParseResult::InvalidNodeType;
    }

    // Find properties list for current config
    // TODO: turn this to CmagTarget method?
    auto propertiesIt = std::find_if(outTarget.properties.begin(), outTarget.properties.end(), [configName](const auto &props) {
        return configName == props.first;
    });
    CmagTarget::Properties *properties = nullptr;
    if (propertiesIt == outTarget.properties.end()) {
        outTarget.properties.push_back({std::string(configName), {}});
        properties = &outTarget.properties.back();
    } else {
        properties = &*propertiesIt; // wtf, why is there no get()?
    }

    // Read properties from json and add them to current config's properties.
    for (auto it = node.begin(); it != node.end(); it++) {
        CmagTargetProperty property = {it.key(), it.value()};
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

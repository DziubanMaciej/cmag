#include "cmag_json_writer.h"

#include <iomanip>

// TODO share this with CmagJsonParser
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

void CmagJsonWriter::writeProject(const CmagProject &project, std::ostream &out) {
    nlohmann::json node{};
    node["globals"] = createGlobalsNode(project.getGlobals());
    for (std::string_view config : project.getConfigs()) {
        const std::string nodeName = std::string("targets") + std::string(config);
        node[nodeName] = createConfigNode(project.getTargets(), config);
    }
    out << std::setw(4) << node;
}

nlohmann::json CmagJsonWriter::createGlobalsNode(const CmagGlobals &globals) {
    nlohmann::json node{};
    node["darkMode"] = globals.darkMode;
    return node;
}

nlohmann::json CmagJsonWriter::createConfigNode(const std::vector<CmagTarget> &targets, std::string_view config) {
    nlohmann::json node = nlohmann::json::array();
    for (const CmagTarget &target : targets) {
        std::optional<nlohmann::json> targetNode = createTargetNode(target, config);
        if (targetNode.has_value()) {
            node.push_back(targetNode.value());
        }
    }
    return node;
}

std::optional<nlohmann::json> CmagJsonWriter::createTargetNode(const CmagTarget &target, std::string_view config) {
    const CmagTarget::Properties *propertiesForCurrentConfig = nullptr;
    for (const CmagTarget::Properties &propertiesForConfig : target.properties) {
        if (propertiesForConfig.first == config) {
            propertiesForCurrentConfig = &propertiesForConfig;
            break;
        }
    }
    if (propertiesForCurrentConfig == nullptr) {
        return {};
    }

    nlohmann::json node{};
    node["name"] = target.name;
    node["type"] = target.type;
    node["properties"] = createPropertiesNode(propertiesForCurrentConfig->second);
    return node;
}

nlohmann::json CmagJsonWriter::createPropertiesNode(const std::vector<CmagTarget::Property> &properties) {
    nlohmann::json node = nlohmann::json::object();
    for (const CmagTarget::Property &property : properties) {
        node[property.first] = property.second;
    }
    return node;
}

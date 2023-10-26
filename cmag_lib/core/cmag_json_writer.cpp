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
    nlohmann::json node = nlohmann::json::object();
    node["globals"] = createGlobalsNode(project.getGlobals());
    node["targets"] = createTargetsNode(project.getTargets());
    out << std::setw(4) << node;
}

nlohmann::json CmagJsonWriter::createGlobalsNode(const CmagGlobals &globals) {
    nlohmann::json node = nlohmann::json::object();
    node["darkMode"] = globals.darkMode;
    return node;
}

nlohmann::json CmagJsonWriter::createTargetsNode(const std::vector<CmagTarget> &targets) {
    nlohmann::json node = nlohmann::json::object();
    for (const CmagTarget &target : targets) {
        node[target.name] = createTargetNode(target);
    }
    return node;
}

nlohmann::json CmagJsonWriter::createTargetNode(const CmagTarget &target) {
    nlohmann::json node = nlohmann::json::object();
    node["type"] = target.type;
    node["configs"] = createConfigsNode(target.configs);
    return node;
}

nlohmann::json CmagJsonWriter::createConfigsNode(const std::vector<CmagTargetConfig> &configs) {
    nlohmann::json node = nlohmann::json::object();
    for (const CmagTargetConfig &config : configs) {
        node[config.name] = createConfigNode(config);
    }
    return node;
}

nlohmann::json CmagJsonWriter::createConfigNode(const CmagTargetConfig &config) {
    nlohmann::json node = nlohmann::json::object();
    for (const CmagTargetProperty &property : config.properties) {
        node[property.name] = property.value;
    }
    return node;
}
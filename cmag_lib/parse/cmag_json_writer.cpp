#include "cmag_json_writer.h"

#include "cmag_lib/parse/enum_serialization.h"

#include <iomanip>

void CmagJsonWriter::writeProject(const CmagProject &project, std::ostream &out) {
    nlohmann::json node = nlohmann::json::object();
    node["globals"] = createGlobalsNode(project.getGlobals());
    node["targets"] = createTargetsNode(project.getTargets());
    out << std::setw(4) << node;
}

nlohmann::json CmagJsonWriter::createGlobalsNode(const CmagGlobals &globals) {
    nlohmann::json node = nlohmann::json::object();
#define WRITE_GLOBAL_FIELD(name) node[#name] = globals.name
    WRITE_GLOBAL_FIELD(darkMode);
    WRITE_GLOBAL_FIELD(selectedConfig);
    WRITE_GLOBAL_FIELD(cmagVersion);
    WRITE_GLOBAL_FIELD(cmakeVersion);
    WRITE_GLOBAL_FIELD(cmakeProjectName);
    WRITE_GLOBAL_FIELD(cmagProjectName);
    WRITE_GLOBAL_FIELD(sourceDir);
    WRITE_GLOBAL_FIELD(buildDir);
    WRITE_GLOBAL_FIELD(generator);
    WRITE_GLOBAL_FIELD(compilerId);
    WRITE_GLOBAL_FIELD(compilerVersion);
    WRITE_GLOBAL_FIELD(os);
#undef WRITE_GLOBAL_FIELD
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
    node["graphical"] = createTargetGraphicalNode(target.graphical);
    return node;
}

nlohmann::json CmagJsonWriter::createTargetGraphicalNode(const CmagTargetGraphicalData &graphicalData) {
    nlohmann::json node = nlohmann::json::object();
    node["x"] = graphicalData.x;
    node["y"] = graphicalData.y;
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

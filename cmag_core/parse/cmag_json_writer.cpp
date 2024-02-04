#include "cmag_json_writer.h"

#include "cmag_core/parse/enum_serialization.h"

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
    WRITE_GLOBAL_FIELD(useFolders);
    WRITE_GLOBAL_FIELD(sourceDir);
    WRITE_GLOBAL_FIELD(buildDir);
    WRITE_GLOBAL_FIELD(generator);
    WRITE_GLOBAL_FIELD(compilerId);
    WRITE_GLOBAL_FIELD(compilerVersion);
    WRITE_GLOBAL_FIELD(os);
    WRITE_GLOBAL_FIELD(cmagProjectName);
#undef WRITE_GLOBAL_FIELD
    node["browser"] = createGlobalValueBrowser(globals.browser);
    node["listDirs"] = createGlobalValueListDirs(globals);
    return node;
}

nlohmann::json CmagJsonWriter::createGlobalValueBrowser(const CmagGlobals::BrowserData &browser) {
    nlohmann::json node = nlohmann::json::object();
#define WRITE_BROWSER_FIELD(name) node[#name] = browser.name
    WRITE_BROWSER_FIELD(needsLayout);
    WRITE_BROWSER_FIELD(autoSaveEnabled);
    WRITE_BROWSER_FIELD(cameraX);
    WRITE_BROWSER_FIELD(cameraY);
    WRITE_BROWSER_FIELD(cameraScale);
    WRITE_BROWSER_FIELD(displayedDependencyType);
    WRITE_BROWSER_FIELD(selectedTabIndex);
    WRITE_BROWSER_FIELD(selectedTargetName);
#undef WRITE_BROWSER_FIELD
    return node;
}

nlohmann::json CmagJsonWriter::createGlobalValueListDirs(const CmagGlobals &globals) {
    nlohmann::json node = nlohmann::json::object();
    for (const CmagListDir &listDir : globals.listDirs) {
        node[listDir.name] = createListDirsNode(listDir, globals);
    }
    return node;
}

nlohmann::json CmagJsonWriter::createListDirsNode(const CmagListDir &listDir, const CmagGlobals &globals) {
    nlohmann::json node = nlohmann::json::array();
    for (const size_t childIndex : listDir.childIndices) {
        const std::string &childName = globals.listDirs[childIndex].name;
        node.push_back(childName);
    }
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
    node["listDir"] = target.listDirName;
    node["isImported"] = target.isImported;
    node["aliases"] = createAliasesNode(target.aliases);
    return node;
}

nlohmann::json CmagJsonWriter::createTargetGraphicalNode(const CmagTargetGraphicalData &graphicalData) {
    nlohmann::json node = nlohmann::json::object();
    node["x"] = graphicalData.x;
    node["y"] = graphicalData.y;
    node["hideConnections"] = graphicalData.hideConnections;
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
nlohmann::json CmagJsonWriter::createAliasesNode(const std::vector<std::string> &aliases) {
    nlohmann::json node = nlohmann::json::array();
    for (const std::string &alias : aliases) {
        node.push_back(alias);
    }
    return node;
}

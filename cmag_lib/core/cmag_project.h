#pragma once

#include <string>
#include <utility>
#include <vector>

enum class CmagTargetType {
    Invalid,
    StaticLibrary,
    ModuleLibrary,
    SharedLibrary,
    ObjectLibrary,
    InterfaceLibrary,
    Executable,
    Utility,
    // TODO ExternalLibrary?

    COUNT,
};

struct CmagGlobals {
    bool darkMode = false;
    std::string cmagVersion = {};
    std::string cmakeVersion = {};
    std::string cmakeProjectName = {};
    std::string cmagProjectName = {};
    std::string sourceDir = {};
    std::string buildDir = {};
    std::string generator = {};
    std::string compilerId = {};
    std::string compilerVersion = {};
    std::string os = {};
};

struct CmagTargetProperty {
    std::string name;
    std::string value;
};

struct CmagTarget;

struct CmagTargetConfig {
    std::string name = {};
    std::vector<CmagTargetProperty> properties = {};

    struct {
        std::vector<const CmagTarget*> linkDependencies = {}; // based on LINK_LIBRARIES
        std::vector<const CmagTarget*> buildDependencies = {}; // based on LINK_LIBRARIES + MANUALLY_ADDED_DEPENDENCIES
    } derived = {};

    void deriveData(const std::vector<CmagTarget> &targets);
    void fixupWithNonEvaled(std::string_view propertyName, std::string_view nonEvaledValue);

private:
    static void fixupLinkLibrariesDirectoryId(std::string &value);
    static void fixupLinkLibrariesGenex(CmagTargetProperty &property, std::string_view nonEvaledValue);
};

struct CmagTargetGraphicalData {
    float x = {};
    float y = {};
};

struct CmagTarget {
    std::string name;
    CmagTargetType type;
    std::vector<CmagTargetConfig> configs;
    CmagTargetGraphicalData graphical = {};
    void *userData = {};

    void deriveData(const std::vector<CmagTarget> &targets);
    CmagTargetConfig &getOrCreateConfig(std::string_view configName);
};

using CmagConfigs = std::vector<std::string>;

class CmagProject {
public:
    CmagProject() = default;

    void addTargetGraphical(std::string_view targetName, float x, float y);
    bool addTarget(CmagTarget &&newTarget);

    void deriveData();

    const auto &getConfigs() const { return configs; }
    const auto &getTargets() const { return targets; }
    auto &getTargets() { return targets; }
    const auto &getGlobals() const { return globals; }
    auto &getGlobals() { return globals; }

private:
    static bool mergeTargets(CmagTarget &dst, CmagTarget &&src);
    void addConfig(std::string_view config);

    CmagConfigs configs = {};
    CmagGlobals globals = {};
    std::vector<CmagTarget> targets = {};
};

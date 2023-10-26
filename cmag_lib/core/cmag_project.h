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
    // TODO ExternalLibrary?
};

struct CmagGlobals {
    bool darkMode = false;
};

struct CmagTargetProperty {
    std::string name;
    std::string value;
};

struct CmagTargetConfig {
    std::string name;
    std::vector<CmagTargetProperty> properties;
};

struct CmagTarget {
    std::string name;
    CmagTargetType type;
    std::vector<CmagTargetConfig> configs;

    CmagTargetConfig &getOrCreateConfig(std::string_view configName);
};

using CmagConfigs = std::vector<std::string>;

class CmagProject {
public:
    CmagProject() = default;

    void addTarget(CmagTarget &&newTarget);

    const auto &getConfigs() const { return configs; }
    const auto &getTargets() const { return targets; }
    const auto &getGlobals() const { return globals; }
    auto &getGlobals() { return globals; }

private:
    void mergeTargets(CmagTarget &dst, CmagTarget &&src);
    void addConfig(std::string_view config);

    CmagConfigs configs = {};
    CmagGlobals globals = {};
    std::vector<CmagTarget> targets = {};
};

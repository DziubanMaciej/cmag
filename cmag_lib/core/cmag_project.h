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

struct CmagTarget {
    using Properties = std::pair<std::string, std::vector<CmagTargetProperty>>; // configName, properties

    std::string name;
    CmagTargetType type;
    std::vector<Properties> properties;
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

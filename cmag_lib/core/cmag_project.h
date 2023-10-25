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

struct CmagTarget {
    using Property = std::pair<std::string, std::string>; // propertyName, propertyValue
    using Properties = std::pair<std::string, std::vector<Property>>; // configName, properties

    std::string name;
    CmagTargetType type;
    std::vector<Properties> properties;
};

class CmagProject {
public:
    CmagProject() = default;

    void addTarget(CmagTarget &&newTarget);

    const auto &getTargets() const { return targets; }
    auto &getGlobals() { return globals; }

private:
    void mergeTargets(CmagTarget &dst, CmagTarget &&src);

    CmagGlobals globals;
    std::vector<CmagTarget> targets = {};
};

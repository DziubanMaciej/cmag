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

struct CmagTarget {
    using Property = std::pair<std::string, std::string>; // name, value

    std::string name;
    CmagTargetType type;
    std::vector<Property> properties;
};

class CmagProject {
public:
    CmagProject() = default;

    void addTarget(CmagTarget &&target);
    void setDarkMode(bool value) { darkMode = value; }

    const auto &getTargets() const { return targets; }
    auto getDarkMode() const { return darkMode; }

private:
    bool darkMode = false;
    std::vector<CmagTarget> targets = {};
};

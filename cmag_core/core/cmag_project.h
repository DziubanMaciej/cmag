#pragma once

#include <string>
#include <utility>
#include <vector>

struct CmagTarget;
class CmagProject;

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
const char *cmagTargetTypeToString(CmagTargetType type);

struct CmagFolder {
    // These structures are built based on values of FOLDER CMake property of all the target in
    // the project. There are no global list of the folders, so we derive their structure from
    // available targets.
    std::string fullName;
    std::string relativeName;
    std::vector<size_t> childIndices = {};
    std::vector<size_t> targetIndices = {};
};

struct CmagListDir {
    std::string name;
    std::vector<size_t> childIndices; // indices within globals.listDirs

    struct {
        std::vector<size_t> targetIndices = {};
    } derived = {};
};

struct CmagGlobals {
    bool darkMode = false;
    std::string selectedConfig = {};
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
    std::vector<CmagListDir> listDirs = {};

    struct {
        std::vector<CmagFolder> folders;
    } derived = {};

private:
    friend CmagProject;
    bool deriveData(const std::vector<CmagTarget> &targets);
    bool deriveDataListDirs(const std::vector<CmagTarget> &targets);
    bool deriveDataFolders(const std::vector<CmagTarget> &targets);
    void insertDerivedTargetWithFolder(size_t targetIndex, const std::vector<std::string_view> &folders);
};

struct CmagTargetProperty {
    std::string name = {};
    std::string value = {};
    bool isConsistent = true; // true if this property has the same value for all other configs
};

struct CmagTargetConfig {
    std::string name = {};
    std::vector<CmagTargetProperty> properties = {};

    struct {
        std::vector<const CmagTarget *> linkDependencies = {};  // based on LINK_LIBRARIES
        std::vector<const CmagTarget *> buildDependencies = {}; // based on LINK_LIBRARIES + MANUALLY_ADDED_DEPENDENCIES
    } derived = {};

    void deriveData(const std::vector<CmagTarget> &targets);
    void fixupWithNonEvaled(std::string_view propertyName, std::string_view nonEvaledValue);
    CmagTargetProperty *findProperty(std::string_view propertyName);
    const CmagTargetProperty *findProperty(std::string_view propertyName) const;

private:
    friend CmagTarget;
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
    std::string listDirName = {};

    const CmagTargetConfig *tryGetConfig(std::string_view configName) const;
    CmagTargetConfig &getOrCreateConfig(std::string_view configName);
    const CmagTargetProperty *getPropertyValue(std::string_view propertyName) const;

private:
    friend CmagProject;
    void deriveData(const std::vector<CmagTarget> &targets);
    void deriveDataPropertyConsistency();
};

using CmagConfigs = std::vector<std::string>;

class CmagProject {
public:
    CmagProject() = default;

    void addTargetGraphical(std::string_view targetName, float x, float y);
    bool addTarget(CmagTarget &&newTarget);

    bool deriveData();

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
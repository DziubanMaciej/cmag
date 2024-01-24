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
    UnknownLibrary, // usually used by CMake modules.
    UnknownTarget,  // This is not an actual CMake target type. We use it, when the target is not visible for the postamble.
    Executable,
    Utility,

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
    bool needsLayout = false;
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
        std::vector<const CmagTarget *> buildDependencies = {};     // based on LINK_LIBRARIES
        std::vector<const CmagTarget *> interfaceDependencies = {}; // based on INTERFACE_LINK_LIBRARIES
        std::vector<const CmagTarget *> manualDependencies = {};    // based on MANUALLY_ADDED_DEPENDENCIES
        std::vector<const CmagTarget *> allDependencies = {};
        std::vector<std::string> unmatchedDependencies = {};
    } derived = {};

    void deriveData(std::vector<CmagTarget> &targets);
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
    bool hideConnections = false;
};

struct CmagTarget {
    std::string name;
    CmagTargetType type;
    std::vector<CmagTargetConfig> configs;
    CmagTargetGraphicalData graphical = {};
    void *userData = {};
    std::string listDirName = {};
    bool isImported = false;
    std::vector<std::string> aliases = {};
    struct {
        bool isReferenced = false; // whether this target is a dependency of some other target
    } derived = {};

    const CmagTargetConfig *tryGetConfig(std::string_view configName) const;
    CmagTargetConfig &getOrCreateConfig(std::string_view configName);
    const CmagTargetProperty *getPropertyValue(std::string_view propertyName) const;
    bool isIgnoredImportedTarget() const;
    bool matchesName(std::string_view nameToMatch) const;

private:
    friend CmagProject;
    void deriveData(std::vector<CmagTarget> &targets);
    void deriveDataPropertyConsistency();
};

using CmagConfigs = std::vector<std::string>;

class CmagProject {
public:
    CmagProject() = default;

    bool addTarget(CmagTarget &&newTarget);
    bool addTargetAlias(std::string_view aliasName, std::string_view aliasedTargetName);

    bool deriveData();

    const auto &getConfigs() const { return configs; }
    const auto &getTargets() const { return targets; }
    auto &getTargets() { return targets; }
    const auto &getGlobals() const { return globals; }
    auto &getGlobals() { return globals; }
    const auto &getUnmatchedDependencies() const { return derived.unmatchedDependencies; }

private:
    void deriveUnmatchedDependencies();
    static bool mergeTargets(CmagTarget &dst, CmagTarget &&src);
    void addConfig(std::string_view config);

    CmagConfigs configs = {};
    CmagGlobals globals = {};
    std::vector<CmagTarget> targets = {};
    struct {
        std::vector<std::string> unmatchedDependencies;
    } derived;
};

#include "cmag_project.h"

#include "cmag_core/utils/string_utils.h"

#include <algorithm>
#include <cstring>
#include <optional>

const char *cmagTargetTypeToString(CmagTargetType type) {
    switch (type) {
    case CmagTargetType::StaticLibrary:
        return "static library";
    case CmagTargetType::ModuleLibrary:
        return "module library";
    case CmagTargetType::SharedLibrary:
        return "shared library";
    case CmagTargetType::ObjectLibrary:
        return "object library";
    case CmagTargetType::InterfaceLibrary:
        return "interface library";
    case CmagTargetType::UnknownLibrary:
        return "unknown library";
    case CmagTargetType::UnknownTarget:
        return "unknown target";
    case CmagTargetType::Executable:
        return "executable";
    case CmagTargetType::Utility:
        return "utility target";
    default:
        return "unknown target";
    }
}

void CmagProject::addTargetGraphical(std::string_view targetName, float x, float y) {
    auto targetIt = std::find_if(targets.begin(), targets.end(), [targetName](const CmagTarget &target) {
        return target.name == targetName;
    });
    if (targetIt == targets.end()) {
        return;
    }

    CmagTargetGraphicalData &graphical = targetIt->graphical;
    graphical.x = x;
    graphical.y = y;
}

bool CmagProject::addTarget(CmagTarget &&newTarget) {
    // Register target's config if we haven't seen it yet.
    for (const CmagTargetConfig &config : newTarget.configs) {
        addConfig(config.name);
    }

    // Look, whether we already have a target of that name. In that case, we will
    // merge them, because it is a definition of the target for a different config.
    for (CmagTarget &existingTarget : targets) {
        if (existingTarget.name == newTarget.name) {
            return mergeTargets(existingTarget, std::move(newTarget));
        }
    }

    // If we hadn't found a matching existing target, we create new one.
    targets.push_back(std::move(newTarget));
    return true;
}

bool CmagProject::mergeTargets(CmagTarget &dst, CmagTarget &&src) {
    if (dst.type != src.type) {
        return false;
    }

    for (CmagTargetConfig &srcConfig : src.configs) {
        auto propertiesIt = std::find_if(dst.configs.begin(), dst.configs.end(), [&srcConfig](const auto &dstConfig) {
            return dstConfig.name == srcConfig.name;
        });
        if (propertiesIt != dst.configs.end()) {
            return false;
        }
    }

    for (CmagTargetConfig &srcConfig : src.configs) {
        dst.configs.push_back(std::move(srcConfig));
    }

    return true;
}

void CmagProject::addConfig(std::string_view config) {
    if (std::find(configs.begin(), configs.end(), config) == configs.end()) {
        configs.emplace_back(config);
    }
}
bool CmagProject::deriveData() {
    for (CmagTarget &target : targets) {
        target.deriveData(targets);
    }
    return globals.deriveData(targets);
}

void CmagTargetConfig::fixupWithNonEvaled(std::string_view propertyName, std::string_view nonEvaledValue) {
    if (propertyName == "LINK_LIBRARIES" || propertyName == "INTERFACE_LINK_LIBRARIES") {
        // Find the property
        auto it = std::find_if(properties.begin(), properties.end(), [propertyName](const CmagTargetProperty &p) {
            return p.name == propertyName;
        });
        if (it == properties.end()) {
            // Technically this should never happen and we should crash, but let's be liberal and ignore this.
            return;
        }

        // Remove directory id from our property.
        fixupLinkLibrariesDirectoryId(it->value);

        // Remove directory id from evaled property value, so we can use it for genex fixup.
        std::string nonEvaledValueFixed = std::string{nonEvaledValue};
        fixupLinkLibrariesDirectoryId(nonEvaledValueFixed);

        // Finaly fixup genexes
        fixupLinkLibrariesGenex(*it, nonEvaledValueFixed);
    }
}

void CmagTargetConfig::fixupLinkLibrariesDirectoryId(std::string &value) {
    // When target_link_libraries() is called in a different directory than add_libraries(), CMake
    // will wrap target XXX with following syntax: ::@(000002F0C2555640);XXX;::@. We have to extract
    // XXX from every instance of this string and remove the wrapping characters.

    size_t currentOffset = 0;

    while (true) {
        const size_t directoryIdStartPos = value.find("::@(", currentOffset);
        if (directoryIdStartPos == std::string::npos) {
            break;
        }

        constexpr const char *directoryIdEndPattern = ");";
        constexpr size_t directoryIdEndPatternLength = std::char_traits<char>::length(directoryIdEndPattern);
        const size_t directoryIdEndPos = value.find(directoryIdEndPattern, directoryIdStartPos);
        if (directoryIdEndPos == std::string::npos) {
            // Unclosed pattern, give up and ignore
            break;
        }

        constexpr const char *entryEndPattern = ";::@";
        constexpr size_t entryEndPatternLength = std::char_traits<char>::length(entryEndPattern);
        const size_t entryEndPos = value.find(entryEndPattern, directoryIdEndPos);
        if (entryEndPos == std::string::npos) {
            // Unclosed pattern, give up and ignore
            break;
        }

        {
            size_t removeStart = entryEndPos;
            size_t removeEnd = entryEndPatternLength;
            value.erase(removeStart, removeEnd);
        }
        {
            size_t removeStart = directoryIdStartPos;
            size_t removeEnd = directoryIdEndPos - directoryIdStartPos + directoryIdEndPatternLength;
            value.erase(removeStart, removeEnd);
        }
    }
}

void CmagTargetConfig::fixupLinkLibrariesGenex(CmagTargetProperty &property, std::string_view nonEvaledValue) {
    // LINK_LIBRARIES and INTERFACE_LINK_LIBRARIES may contain a generator expression $<LINK_ONLY:XXX>.
    // This resolves to XXX, when evaluating with $<GENEX_EVAL>, even though we would expect it
    // to be empty. As a workaround we dump both evaled and non-evaled versions. Non evaled version can
    // to look up which entries are in form $<LINK_ONLY:XXX>. Then we can remove these entries from
    // evaled version.

    std::string &evaledValue = property.value;

    // Early return if we have no genexes
    if (nonEvaledValue.find('$') == std::string::npos) {
        return;
    }

    // Find all values wrapped with $<LINK_ONLY>
    const char *prefix = "$<LINK_ONLY:";
    const size_t prefixLength = strlen(prefix);
    size_t genexDepth = 0;
    size_t elementIndex = 0;
    bool isPrefixMatched = false;
    std::vector<size_t> elementIndicesToRemove = {};
    for (size_t positionInNonEvaled = 0; positionInNonEvaled < nonEvaledValue.length(); positionInNonEvaled++) {
        const char currentChar = nonEvaledValue[positionInNonEvaled];
        const char previousChar = positionInNonEvaled > 0 ? nonEvaledValue[positionInNonEvaled - 1] : '\0';

        // Update regex depth
        if (previousChar == '$' && currentChar == '<') {
            genexDepth++;
            continue;
        }
        if (previousChar != '\\' && currentChar == '>') {
            if (genexDepth == 0) {
                // Malformed genex, don't try to save it
                return;
            }

            genexDepth--;
            if (genexDepth == 0 && isPrefixMatched) {
                elementIndicesToRemove.push_back(elementIndex);
                isPrefixMatched = false;
            }
            continue;
        }

        // Update element index
        if (previousChar != '\\' && currentChar == ';' && genexDepth == 0) {
            elementIndex++;
            continue;
        }

        // Check if we're inside our cursed genex
        if (positionInNonEvaled >= prefixLength && genexDepth == 1 && !isPrefixMatched) {
            std::string_view currentPrefix = nonEvaledValue.substr(positionInNonEvaled - prefixLength, prefixLength);
            if (prefix == currentPrefix) {
                isPrefixMatched = true;
            }
        }
    }

    // Remove all entries with detected $<LINK_ONLY> genex by their index
    elementIndex = 0;
    size_t elementStartPosition = 0;
    for (size_t positionInEvaled = 0; positionInEvaled < evaledValue.length() + 1; positionInEvaled++) {
        const char currentChar = evaledValue.data()[positionInEvaled];
        if (currentChar != ';' && currentChar != '\0') {
            continue;
        }

        if (std::find(elementIndicesToRemove.begin(), elementIndicesToRemove.end(), elementIndex) != elementIndicesToRemove.end()) {
            std::size_t removeStart = elementStartPosition;
            std::size_t removeEnd = positionInEvaled;
            if (removeStart > 0 && evaledValue[removeStart - 1] == ';') {
                removeStart--;
            } else if (removeEnd < evaledValue.length() - 1 && evaledValue[removeEnd] == ';') {
                removeEnd++;
            }
            evaledValue.erase(removeStart, removeEnd - removeStart);

            positionInEvaled = elementStartPosition;
        } else {
            elementStartPosition = positionInEvaled + 1;
        }

        elementIndex++;
    }
}
void CmagTargetConfig::deriveData(std::vector<CmagTarget> &targets) {
    auto addTargetsToVector = [&targets](std::vector<std::string_view> &strings, std::vector<const CmagTarget *> &outList) {
        for (std::string_view string : strings) {
            auto it = std::find_if(targets.begin(), targets.end(), [string](const CmagTarget &target) {
                return target.name == string;
            });
            if (it != targets.end()) {
                it->derived.isReferenced = true;
                outList.push_back(&*it);
            }
        }
    };

    if (auto property = findProperty("LINK_LIBRARIES"); property != nullptr) {
        std::vector<std::string_view> dependencies = splitCmakeListString(property->value, false);
        addTargetsToVector(dependencies, derived.linkDependencies);
        derived.buildDependencies = derived.linkDependencies;
    }

    if (auto property = findProperty("MANUALLY_ADDED_DEPENDENCIES"); property != nullptr) {
        std::vector<std::string_view> dependencies = splitCmakeListString(property->value, false);
        addTargetsToVector(dependencies, derived.buildDependencies);
    }
}

CmagTargetProperty *CmagTargetConfig::findProperty(std::string_view propertyName) {
    auto it = std::find_if(properties.begin(), properties.end(), [propertyName](const CmagTargetProperty &p) {
        return p.name == propertyName;
    });
    if (it != properties.end()) {
        return &*it;
    }
    return nullptr;
}

const CmagTargetProperty *CmagTargetConfig::findProperty(std::string_view propertyName) const {
    auto it = std::find_if(properties.begin(), properties.end(), [propertyName](const CmagTargetProperty &p) {
        return p.name == propertyName;
    });
    if (it != properties.end()) {
        return &*it;
    }
    return nullptr;
}

const CmagTargetConfig *CmagTarget::tryGetConfig(std::string_view configName) const {
    auto configIt = std::find_if(configs.begin(), configs.end(), [configName](const auto &config) {
        return configName == config.name;
    });
    if (configIt == configs.end()) {
        return nullptr;
    } else {
        return &*configIt;
    }
}

CmagTargetConfig &CmagTarget::getOrCreateConfig(std::string_view configName) {
    auto propertiesIt = std::find_if(configs.begin(), configs.end(), [configName](const auto &config) {
        return configName == config.name;
    });
    if (propertiesIt == configs.end()) {
        configs.push_back({std::string(configName), {}, {}});
        return configs.back();
    } else {
        return *propertiesIt;
    }
}

bool CmagGlobals::deriveData(const std::vector<CmagTarget> &targets) {
    return deriveDataListDirs(targets) && deriveDataFolders(targets);
}

bool CmagGlobals::deriveDataListDirs(const std::vector<CmagTarget> &targets) {
    for (size_t targetIndex = 0u; targetIndex < targets.size(); targetIndex++) {
        const CmagTarget &target = targets[targetIndex];

        bool found = false;
        for (CmagListDir &listDir : listDirs) {
            if (listDir.name != target.listDirName) {
                continue;
            }

            listDir.derived.targetIndices.push_back(targetIndex);
            found = true;
            break;
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

bool CmagGlobals::deriveDataFolders(const std::vector<CmagTarget> &targets) {
    derived.folders.clear();
    derived.folders.push_back(CmagFolder{"", ""});

    for (size_t targetIndex = 0u; targetIndex < targets.size(); targetIndex++) {
        const CmagTarget &target = targets[targetIndex];
        const CmagTargetProperty *property = target.getPropertyValue("FOLDER");
        if (property == nullptr) {
            derived.folders[0].targetIndices.push_back(targetIndex);
            continue; // this shouldn't really happen, since we force FOLDER to be dumped, but let's be liberal.
        }
        if (!property->isConsistent) {
            return false;
        }
        const std::vector<std::string_view> folders = splitStringByChar(property->value, false, '/');
        insertDerivedTargetWithFolder(targetIndex, folders);
    }
    return true;
}

void CmagGlobals::insertDerivedTargetWithFolder(size_t targetIndex, const std::vector<std::string_view> &folders) {
    // Initialize the current folder we are looking at. Zero is the root folder.
    size_t currentFolderIndex = 0;

    // Find leaf folder containing the target
    for (std::string_view currentTargetFolderName : folders) {
        // Get index of the child folder
        std::optional<size_t> matchingChildIndex = {};
        for (size_t childIndex : derived.folders[currentFolderIndex].childIndices) {
            if (derived.folders[childIndex].relativeName == currentTargetFolderName) {
                matchingChildIndex = childIndex;
                break;
            }
        }

        // If index of child folder was not found, we have to create it
        if (!matchingChildIndex.has_value()) {
            CmagFolder &currentFolder = derived.folders[currentFolderIndex];

            matchingChildIndex = derived.folders.size();
            currentFolder.childIndices.push_back(matchingChildIndex.value());

            std::string relativeName = std::string(currentTargetFolderName);
            std::string fullName;
            if (currentFolder.fullName.empty()) {
                fullName = relativeName;
            } else {
                fullName = currentFolder.fullName + "/" + relativeName;
            }
            derived.folders.push_back(CmagFolder{std::move(fullName), std::move(relativeName)});
        }

        // Advance to the child folder
        currentFolderIndex = matchingChildIndex.value();
    }

    // Assign target's index to the folder
    derived.folders[currentFolderIndex].targetIndices.push_back(targetIndex);
}

void CmagTarget::deriveData(std::vector<CmagTarget> &targets) {
    for (CmagTargetConfig &config : configs) {
        config.deriveData(targets);
    }
    deriveDataPropertyConsistency();
}

void CmagTarget::deriveDataPropertyConsistency() {
    if (configs.size() < 2) {
        return;
    }

    std::vector<CmagTargetProperty *> cachedProperties(configs.size());

    // This loop is written with the assumption that all configs contain the same properties. W
    // will loop through properties of a zeroth config and assume they will also be found on other
    // configs.
    // It may work incorrectly if they are different. This scenario could happen only when merging
    // multiple single-generator projects.
    // TODO: disallow different properties for configs in cmag --merge
    CmagTargetConfig &defaultConfig = configs[0];
    for (auto &defaultConfigProperty : defaultConfig.properties) {
        std::fill(cachedProperties.begin(), cachedProperties.end(), nullptr);
        cachedProperties[0] = &defaultConfigProperty;

        // Search for the same property in rest of configs and verify whether they are the same
        bool isConsistent = true;
        for (size_t configIndex = 1u; configIndex < configs.size(); configIndex++) {
            CmagTargetProperty *currentConfigProperty = configs[configIndex].findProperty(defaultConfigProperty.name);
            cachedProperties[configIndex] = currentConfigProperty;

            // this situation shouldn't normally happen for correctly generated cmag projects, but let's
            // handle it to be robust.
            if (currentConfigProperty == nullptr) {
                isConsistent = false;
                continue;
            }

            if (defaultConfigProperty.value != currentConfigProperty->value) {
                isConsistent = false;
            }
        }

        // Set the boolean value for cachedProperties of all configs.
        for (CmagTargetProperty *property : cachedProperties) {
            if (property == nullptr) {
                continue;
            }
            property->isConsistent = isConsistent;
        }
    }
}
const CmagTargetProperty *CmagTarget::getPropertyValue(std::string_view propertyName) const {
    // Select any config. User of this method should check whether the acquired property
    // is consistent.
    return configs[0].findProperty(propertyName);
}

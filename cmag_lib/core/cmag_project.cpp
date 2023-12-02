#include "cmag_project.h"

#include "cmag_lib/utils/string_utils.h"

#include <algorithm>
#include <cstring>

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
void CmagProject::deriveData() {
    for (CmagTarget &target : targets) {
        target.deriveData(targets);
    }
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
void CmagTargetConfig::deriveData(const std::vector<CmagTarget> &targets) {
    auto addTargetsToVector = [&targets](std::vector<std::string_view> &strings, std::vector<const CmagTarget *> &outList) {
        for (std::string_view string : strings) {
            auto it = std::find_if(targets.begin(), targets.end(), [string](const CmagTarget &target) {
                return target.name == string;
            });
            if (it != targets.end()) {
                outList.push_back(&*it);
            }
        }
    };

    auto findProperty = [this](std::string_view propertyName) {
        return std::find_if(properties.begin(), properties.end(), [propertyName](const CmagTargetProperty &p) {
            return p.name == propertyName;
        });
    };

    if (auto it = findProperty("LINK_LIBRARIES"); it != properties.end()) {
        std::vector<std::string_view> dependencies = splitCmakeListString(it->value, false);
        addTargetsToVector(dependencies, derived.linkDependencies);
        derived.buildDependencies = derived.linkDependencies;
    }

    if (auto it = findProperty("MANUALLY_ADDED_DEPENDENCIES"); it != properties.end()) {
        std::vector<std::string_view> dependencies = splitCmakeListString(it->value, false);
        addTargetsToVector(dependencies, derived.buildDependencies);
    }
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
void CmagTarget::deriveData(const std::vector<CmagTarget> &targets) {
    for (CmagTargetConfig &config : configs) {
        config.deriveData(targets);
    }
}

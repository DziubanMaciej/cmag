#include "cmag_project.h"

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
        configs.push_back(std::string(config));
    }
}

void CmagTargetConfig::fixupLinkLibraries(std::string_view propertyName, std::string_view nonEvaledValue) {
    // LINK_LIBRARIES and INTERFACE_LINK_LIBRARIES may contain a generator expression $<LINK_ONLY:XXX>.
    // This resolves to XXX, when evaluating with $<GENEX_EVAL>, even though we would expect it
    // to be empty. As a workaround we dump both evaled and non-evaled versions. Non evaled version can
    // to look up which entries are in form $<LINK_ONLY:XXX>. Then we can remove these entries from
    // evaled version.

    // Find the property
    auto it = std::find_if(properties.begin(), properties.end(), [propertyName](const CmagTargetProperty &p) {
        return p.name == propertyName;
    });
    if (it == properties.end()) {
        // Technically this should never happen and we should crash, but let's be liberal and ignore this.
        return;
    }
    std::string &evaledValue = it->value;

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

CmagTargetConfig &CmagTarget::getOrCreateConfig(std::string_view configName) {
    auto propertiesIt = std::find_if(configs.begin(), configs.end(), [configName](const auto &config) {
        return configName == config.name;
    });
    if (propertiesIt == configs.end()) {
        configs.push_back({std::string(configName), {}});
        return configs.back();
    } else {
        return *propertiesIt;
    }
}

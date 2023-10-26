#include "cmag_project.h"

#include <algorithm>

void CmagProject::addTarget(CmagTarget &&newTarget) {
    // Register target's config if we haven't seen it yet.
    for (const CmagTargetConfig &config : newTarget.configs) {
        addConfig(config.name);
    }

    // Look, whether we already have a target of that name. In that case, we will
    // merge them, because it is a definition of the target for a different config.
    for (CmagTarget &existingTarget : targets) {
        if (existingTarget.name == newTarget.name) {
            mergeTargets(existingTarget, std::move(newTarget));
            return;
        }
    }

    // If we hadn't found a matching existing target, we create new one.
    targets.push_back(std::move(newTarget));
}

void CmagProject::mergeTargets(CmagTarget &dst, CmagTarget &&src) {
    // TODO check for config duplicates. What should happen then?
    // TODO check for matching type. What should happen if they don't match?

    for (CmagTargetConfig &srcConfig : src.configs) {
        dst.configs.push_back(std::move(srcConfig));
    }
}

void CmagProject::addConfig(std::string_view config) {
    if (std::find(configs.begin(), configs.end(), config) == configs.end()) {
        configs.push_back(std::string(config));
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

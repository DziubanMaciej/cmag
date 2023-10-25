#include "cmag_project.h"

void CmagProject::addTarget(CmagTarget &&newTarget) {
    // Register target's config if we haven't seen it yet.
    for (const CmagTarget::Properties &properties : newTarget.properties) {
        addConfig(properties.first);
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

    for (CmagTarget::Properties &srcProperties : src.properties) {
        dst.properties.push_back(std::move(srcProperties));
    }
}

void CmagProject::addConfig(std::string_view config) {
    if (std::find(configs.begin(), configs.end(), config) == configs.end()) {
        configs.push_back(std::string(config));
    }
}

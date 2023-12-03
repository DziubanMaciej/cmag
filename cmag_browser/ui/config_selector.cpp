#include "config_selector.h"

#include "cmag_lib/core/cmag_project.h"

#include <imgui.h>

ConfigSelector::ConfigSelector(CmagProject &project) : project(project),
                                                       currentSelection(0),
                                                       selectionsCount(static_cast<int>(project.getConfigs().size())),
                                                       configs(std::make_unique<const char *[]>(selectionsCount)) {
    for (int configIndex = 0; configIndex < selectionsCount; configIndex++) {
        configs[configIndex] = project.getConfigs()[configIndex].c_str();
        if (configs[configIndex] == project.getGlobals().selectedConfig) {
            currentSelection = static_cast<int>(configIndex);
        }
    }
}
void ConfigSelector::render() {
    ImGui::Combo("##ConfigSelection", &currentSelection, configs.get(), selectionsCount);
}

std::string_view ConfigSelector::getCurrentConfig() {
    return configs[currentSelection];
}

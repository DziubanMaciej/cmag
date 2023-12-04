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
void ConfigSelector::render(float width) {
    if (width != 0) {
        ImGui::SetNextItemWidth(width);
    }
    if (selectionsCount == 1) {
        ImGui::BeginDisabled();
    }
    ImGui::Combo("##ConfigSelection", &currentSelection, configs.get(), selectionsCount);
    if (selectionsCount == 1) {
        ImGui::EndDisabled();

        // TODO wrap tooltips in some shared helper
        if (ImGui::BeginItemTooltip()) {
            const int wrapWidth = 300;
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapWidth);
            ImGui::Text("Only %s config is available. This is common for single config generators. See output of cmag -h about --merge option to be able to compare multiple configs", configs[0]);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

std::string_view ConfigSelector::getCurrentConfig() {
    return configs[currentSelection];
}

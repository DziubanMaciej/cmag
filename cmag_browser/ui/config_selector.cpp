#include "config_selector.h"

#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_lib/core/cmag_project.h"

#include <imgui.h>

ConfigSelector::ConfigSelector(const CmagBrowserTheme &theme, CmagProject &project)
    : theme(theme),
      project(project),
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
void ConfigSelector::render(float width, bool skipTooltip) {
    if (width != 0) {
        ImGui::SetNextItemWidth(width);
    }
    if (selectionsCount == 1) {
        ImGui::BeginDisabled();
    }
    ImGui::Combo("##ConfigSelection", &currentSelection, configs.get(), selectionsCount);
    if (selectionsCount == 1) {
        ImGui::EndDisabled();
        if (!skipTooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            renderTooltip();
        }
    }
}

void ConfigSelector::renderTooltip() {
    if (selectionsCount == 1) {
        const char *format = "Only %s config is available. This is common for single config generators. See output of cmag -h about --merge option to be able to compare multiple configs";
        char buffer[256];
        snprintf(buffer, sizeof(buffer), format, configs[0]);

        Tooltip::renderTooltip(theme, buffer, nullptr);
    }
}

std::string_view ConfigSelector::getCurrentConfig() {
    return configs[currentSelection];
}

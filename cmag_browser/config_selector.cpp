#include "config_selector.h"

#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/core/cmag_project.h"

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

    if (selectionsCount == 1) {
        const char *format = "Only %s config is available. This is common for single config generators. See output of cmag -h about --merge option to be able to compare multiple configs";
        snprintf(singleConfigGeneratorWarning, sizeof(singleConfigGeneratorWarning), format, configs[0]);
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
    }
}

void ConfigSelector::renderTooltipLastItem() {
    if (singleConfigGeneratorWarning[0] != '\0') {
        TooltipBuilder(theme)
            .setHoverLastItem()
            .addText(singleConfigGeneratorWarning)
            .execute();
    }
}

void ConfigSelector::renderTooltipRect(ImVec2 min, ImVec2 max) {
    if (singleConfigGeneratorWarning[0] != '\0') {
        TooltipBuilder(theme)
            .setHoverRect(min, max)
            .addText(singleConfigGeneratorWarning)
            .execute();
    }
}

std::string_view ConfigSelector::getCurrentConfig() {
    return configs[currentSelection];
}

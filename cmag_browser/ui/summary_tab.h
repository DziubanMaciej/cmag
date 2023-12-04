#pragma once

#include <imgui/imgui.h>
#include <string>

class CmagProject;
class ConfigSelector;

class SummaryTab {
public:
    SummaryTab(CmagProject &project, ConfigSelector &configSelector);
    void render();

private:
    static void renderTableRowString(const char *name, const std::string &value, const char *tooltip, const char *tooltipHyperlink = nullptr);
    static void renderTooltip(const char *tooltip, const char *tooltipHyperlink);
    void renderTableRowSelectedConfig();
    static bool isRectHovered(ImVec2 min, ImVec2 max);

    static void renderTableRowSpacer();

    CmagProject &project;
    ConfigSelector &configSelector;
    std::string compiler;
};

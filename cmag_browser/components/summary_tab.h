#pragma once

#include <imgui/imgui.h>
#include <string>

class CmagProject;
class ConfigSelector;
class CmagBrowserTheme;

class SummaryTab {
public:
    SummaryTab(const CmagBrowserTheme &theme, CmagProject &project, ConfigSelector &configSelector);
    void render();

private:
    void renderTableRowString(const char *name, const std::string &value, const char *tooltip, const char *tooltipHyperlink = nullptr);
    void renderTableRowSelectedConfig();
    static void renderTableRowSpacer();

    const CmagBrowserTheme &theme;
    CmagProject &project;
    ConfigSelector &configSelector;
    std::string compiler;
};

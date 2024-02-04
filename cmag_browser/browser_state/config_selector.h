#pragma once

#include <imgui/imgui.h>
#include <memory>
#include <string_view>

class CmagProject;
class CmagBrowserTheme;
class ProjectSaver;

class ConfigSelector {
public:
    ConfigSelector(const CmagBrowserTheme &theme, CmagProject &project, ProjectSaver &projectSaver);

    void render(float width);
    void renderTooltipLastItem();
    void renderTooltipRect(ImVec2 min, ImVec2 max);
    std::string_view getCurrentConfig();

private:
    const CmagBrowserTheme &theme;
    CmagProject &project;
    ProjectSaver &projectSaver;
    int currentSelection;
    int selectionsCount;
    std::unique_ptr<const char *[]> configs;
    char singleConfigGeneratorWarning[256] = {};
};

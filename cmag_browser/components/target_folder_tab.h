#pragma once

#include <imgui/imgui.h>
#include <string_view>

class CmagBrowserTheme;
class CmagProject;
struct CmagTarget;
struct CmagFolder;
class TargetGraphTab;

class TargetFolderTab {
public:
    TargetFolderTab(const CmagBrowserTheme &theme, CmagProject &project, TargetGraphTab &targetGraphTab);

    void render();

private:
    void renderFolder(bool renderSelf, const CmagFolder &folder);
    void renderTarget(CmagTarget &target);

    const CmagBrowserTheme &theme;
    CmagProject &project;
    TargetGraphTab &targetGraphTab;
};

#pragma once

#include <imgui/imgui.h>
#include <string_view>

class CmagBrowserTheme;
class CmagProject;
struct CmagTarget;
struct CmagListDir;
class TargetGraphTab;

class ListDirTab {
public:
    ListDirTab(const CmagBrowserTheme &theme, CmagProject &project, TargetGraphTab &targetGraphTab);

    void render();

private:
    void renderOptions();
    void renderListDir(const char *parentName, const CmagListDir &listDir);
    void renderTarget(CmagTarget &target);
    void renderTooltip(const std::string &currentName);
    static const char *deriveRelativeName(const char *parentName, const std::string &currentName);

    const CmagBrowserTheme &theme;
    CmagProject &project;
    TargetGraphTab &targetGraphTab;
    bool showIgnoredTargets = false;
};

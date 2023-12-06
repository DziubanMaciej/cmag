#pragma once

#include <imgui/imgui.h>
#include <string_view>

class CmagBrowserTheme;
class CmagProject;
struct CmagListDir;

class ListDirTab {
public:
    ListDirTab(const CmagBrowserTheme &theme, const CmagProject &project);

    void render();

private:
    void renderListDir(const char *parentName, const CmagListDir &listDir);
    void renderTooltip(const std::string &currentName);
    static const char *deriveRelativeName(const char *parentName, const std::string &currentName);

    const CmagBrowserTheme &theme;
    const CmagProject &project;
};

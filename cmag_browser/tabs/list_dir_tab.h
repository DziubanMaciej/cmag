#pragma once

#include "cmag_browser/browser_state.h"

#include <imgui/imgui.h>
#include <string_view>

struct CmagTarget;
struct CmagListDir;

class ListDirTab {
public:
    explicit ListDirTab(BrowserState &browser);

    void render();

private:
    void renderOptions();
    void renderListDir(const char *parentName, const CmagListDir &listDir);
    void renderTarget(CmagTarget &target);
    void renderTooltip(const std::string &currentName);
    static const char *deriveRelativeName(const char *parentName, const std::string &currentName);

    BrowserState &browser;
    bool showIgnoredTargets = false;
};

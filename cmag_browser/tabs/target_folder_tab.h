#pragma once

#include "cmag_browser/browser_state.h"

#include <imgui/imgui.h>
#include <string_view>

class CmagBrowserTheme;
class CmagProject;
struct CmagTarget;
struct CmagFolder;
class TargetGraphTab;

class TargetFolderTab {
public:
    TargetFolderTab(BrowserState &browser, TargetGraphTab &targetGraphTab);

    void render();

private:
    void renderHeaders();
    void renderHeadersWarnings();
    void renderFolder(bool renderSelf, const CmagFolder &folder);
    void renderTarget(CmagTarget &target);

    BrowserState &browser;
    TargetGraphTab &targetGraphTab;
    bool showIgnoredTargets = false;
};

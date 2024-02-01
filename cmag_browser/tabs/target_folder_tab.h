#pragma once

#include "cmag_browser/browser_state.h"

#include <imgui/imgui.h>
#include <string_view>

struct CmagTarget;
struct CmagFolder;

class TargetFolderTab {
public:
    explicit TargetFolderTab(BrowserState &browser);

    void render();

private:
    void renderHeaders();
    void renderHeadersWarnings();
    void renderFolder(bool renderSelf, const CmagFolder &folder);
    void renderTarget(CmagTarget &target);

    BrowserState &browser;
    bool showIgnoredTargets = false;
};

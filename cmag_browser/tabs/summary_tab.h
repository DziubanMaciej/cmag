#pragma once

#include "cmag_browser/browser_state.h"

#include <imgui/imgui.h>
#include <string>

struct CmagGlobals;

class SummaryTab {
public:
    SummaryTab(BrowserState &browser);
    void render();

private:
    void renderTableRowString(const char *name, const std::string &value, const char *tooltip, const char *tooltipHyperlink = nullptr);
    void renderTableRowSelectedConfig();
    static void renderTableRowSpacer();
    static std::string createCompilerString(const CmagGlobals &globals);

    BrowserState &browser;
    std::string compiler;
};

#pragma once

#include "cmag_browser/browser_state/browser_state.h"

#include <imgui/imgui.h>
#include <string>

struct CmagGlobals;

class SummaryTab {
public:
    explicit SummaryTab(BrowserState &browser);
    void render();

private:
    void renderSectionHeader(const char *name, const char *tooltip);

    void renderRowLabel(const char *name);
    void renderRowString(const char *name, const std::string &value, const char *tooltip, const char *tooltipHyperlink = nullptr);
    void renderRowConfigSelector();
    void renderRowSave();
    void renderRowAutoSave();

    static std::string createCompilerString(const CmagGlobals &globals);

    BrowserState &browser;
    std::string compiler;
    float firstColumnWidth = 0;
};

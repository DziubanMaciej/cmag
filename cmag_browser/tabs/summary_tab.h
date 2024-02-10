#pragma once

#include "cmag_browser/browser_state/browser_state.h"

#include <imgui/imgui.h>
#include <string>

struct CmagGlobals;

struct Section;

class SummaryTab {
public:
    explicit SummaryTab(BrowserState &browser);
    void render();

private:
    Section renderSectionHeader(const char *name, const char *tooltip);

    void renderRowLabel(const char *name);
    void renderRowString(const char *name, const std::string &value, const char *tooltip, const char *tooltipHyperlink = nullptr);
    void renderRowConfigSelector();
    void renderRowSave();
    void renderRowAutoSave();

    static std::string createCompilerString(const CmagGlobals &globals);

    const float sectionIndentSize = 8.f;
    const float marginBetweenSections = 15.f;
    BrowserState &browser;
    std::string compiler;
    float firstColumnWidth = 0;
};

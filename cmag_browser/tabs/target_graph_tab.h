#pragma once

#include "cmag_browser/browser_state.h"
#include "cmag_browser/target_graph/target_graph.h"

#include <imgui/imgui.h>

class TargetGraphTab {
public:
    TargetGraphTab(BrowserState &browser, bool showDebugWidgets);

    void render(ImGuiIO &io);

private:
    void renderSidePane(float width);
    void renderSidePaneSlider(const char *label, float width, float min, float max, float *value);
    void renderSidePaneDependencyTypeSelection(float width);
    void renderSidePaneHideConnectionsButton(float width);
    void renderPropertyPopup();
    void renderPropertyTable(float width);
    void renderPropertyTablePopup(const CmagTargetProperty &property, bool showValue) const;
    void renderGraph(ImGuiIO &io);

    void scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property);

    BrowserState &browser;
    TargetGraph targetGraph;
    CmakeDependencyType dependencyTypeSelected = CmakeDependencyType::DEFAULT;
    bool forceSelection = false;
    bool showDebugWidgets;

    bool showDemoWindow = false;
    bool showStyleSelector = false;
    struct {
        bool shouldBeOpen = false;
        bool isOpen = false;
        const CmagTargetProperty *property = nullptr;
        std::vector<std::string_view> propertyValueList;
    } popup;
};

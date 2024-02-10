#pragma once

#include "cmag_browser/browser_state/browser_state.h"
#include "cmag_browser/target_graph/target_graph.h"

#include <imgui/imgui.h>

class TargetGraphTab {
public:
    TargetGraphTab(BrowserState &browser, bool showDebugWidgets);

    void render(ImGuiIO &io);

private:
    void renderSidePane(float width);
    void renderSidePaneSectionDebug();
    void renderSidePaneSectionView();
    void renderSidePaneSectionTarget();

    void renderSidePaneSlider(const char *label, float min, float max, float *value);
    void renderSidePaneDependencyTypeSelection();

    void renderPropertyPopup();
    void renderPropertyTable(const CmagTarget *selectedTarget);
    void renderPropertyTablePopup(const CmagTargetProperty &property, ImVec2 cellMin, ImVec2 cellMax, bool showValue) const;
    void renderGraph(ImGuiIO &io);
    void renderConnectionPopup(const TargetGraph::ConnectionData *connection);
    void renderTargetPopup(const ImGuiIO &io, CmagTarget *target);

    void scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property, ImVec2 cellMin, ImVec2 cellMax);

    const float sectionIndentSize = 8.f;
    const float marginBetweenSections = 15.f;
    BrowserState &browser;
    TargetGraph targetGraph;
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

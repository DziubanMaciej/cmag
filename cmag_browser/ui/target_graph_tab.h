#pragma once

#include "cmag_browser/ui/target_graph/target_graph.h"

struct ImGuiIO;

struct TargetGraphTab {
public:
    TargetGraphTab(std::vector<CmagTarget> &targets);

    void render(ImGuiIO &io);

private:
    void renderSidePane(float width);
    void renderPropertyPopup();
    void renderPropertyTable(float width);
    void renderGraph(ImGuiIO &io);

    void scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property);

    TargetGraph targetGraph;
    bool showDemoWindow = false;
    struct {
        bool shouldBeOpen = false;
        bool isOpen = false;
        const CmagTargetProperty *property = nullptr;
        std::vector<std::string_view> propertyValueList;
    } popup;
};

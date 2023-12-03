#pragma once

#include "cmag_browser/ui/target_graph/target_graph.h"

#include <imgui/imgui.h>

struct TargetGraphTab {
public:
    explicit TargetGraphTab(std::vector<CmagTarget> &targets);

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

    struct {
        const ImColor propertyName = ImColor::HSV(0.14f, 0.6f, 0.6f);
        const ImColor inconsistentPropertyName = ImColor::HSV(0.9f, 0.6f, 0.6f);
    } colors;
};

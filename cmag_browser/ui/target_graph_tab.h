#pragma once

#include "cmag_browser/ui/target_graph/target_graph.h"

#include <imgui/imgui.h>

class ConfigSelector;

struct TargetGraphTab {
public:
    TargetGraphTab(CmagProject &project, ConfigSelector &configSelector);

    void render(ImGuiIO &io);

private:
    void renderSidePane(float width);
    void renderSidePaneSlider(const char *label, float width, float min, float max, float *value);
    void renderSidePaneDependencyTypeSelection(float width);
    void renderPropertyPopup();
    void renderPropertyTable(float width);
    void renderGraph(ImGuiIO &io);

    void scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property);

    TargetGraph targetGraph;
    ConfigSelector &configSelector;
    int dependencyTypeComboSelection = static_cast<int>(CmakeDependencyType::DEFAULT);

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

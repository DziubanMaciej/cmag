#pragma once

#include "cmag_browser/ui/target_graph.h"

struct ImGuiIO;

struct TargetGraphTab {
public:
    TargetGraphTab(std::vector<CmagTarget> &targets);

    void render(ImGuiIO &io);

private:
    void renderSidePane(float width);
    void renderPropertyTable(float width);
    void renderGraph(ImGuiIO &io);

    TargetGraph targetGraph;
    bool showDemoWindow = false;
};

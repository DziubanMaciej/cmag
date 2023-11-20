#include "target_graph_tab.h"

#include <imgui/imgui.h>

TargetGraphTab::TargetGraphTab(std::vector<CmagTarget> &targets) : targetGraph(targets) {}

void TargetGraphTab::render(ImGuiIO &io) {
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float sidePaneWidth = windowWidth * 0.2f;

    ImGui::BeginGroup();
    renderSidePane(sidePaneWidth);
    ImGui::EndGroup();
    ImGui::SameLine();

    renderGraph(io);
}

void TargetGraphTab::renderSidePane(float width) {
    ImGui::Checkbox("Demo Window", &showDemoWindow);
    ImGui::Button("Dummy button 1");
    ImGui::Button("Dummy button 2");
    ImGui::Button("Dummy button 3");
    renderPropertyTable(width);

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}

void TargetGraphTab::renderPropertyTable(float width) {
    ImVec2 propertyTableSize = ImGui::GetContentRegionAvail();
    propertyTableSize.x = width;
    const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders;

    if (ImGui::BeginTable("Table populating", 2, tableFlags, propertyTableSize)) {
        CmagTarget *selectedTarget = targetGraph.getSelectedTarget();
        if (selectedTarget != nullptr) {
            for (const CmagTargetProperty &property : selectedTarget->configs[0].properties) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(property.name.c_str());
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip(property.name.c_str());
                }
                ImGui::TableNextColumn();
                ImGui::Text(property.value.c_str());
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip(property.value.c_str());
                }
            }
        }
    }
    ImGui::EndTable();
}

void TargetGraphTab::renderGraph(ImGuiIO &io) {
    ImVec2 space = ImGui::GetContentRegionAvail();
    if (space.x > 0 && space.y > 0) {
        int targetGraphW = static_cast<int>(space.x);
        int targetGraphH = static_cast<int>(space.y);

        targetGraph.update(io);
        targetGraph.render(targetGraphW, targetGraphH);
        ImGui::Image((void *)(intptr_t)targetGraph.getTexture(), space);
        const ImVec2 pos = ImGui::GetItemRectMin();
        targetGraph.savePosition(static_cast<size_t>(pos.x), static_cast<size_t>(pos.y));
    }
}

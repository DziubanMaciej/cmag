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

    renderPropertyPopup();
    renderPropertyTable(width);

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}

void TargetGraphTab::renderPropertyPopup() {
    constexpr const char *popupName = "propertyPopup";

    if (popup.shouldBeOpen && !popup.isOpen) {
        ImGui::OpenPopup(popupName);
        popup.isOpen = true;
    }

    if (ImGui::BeginPopup(popupName)) {
        ImGui::Text("Property %s", popup.property->name.c_str());
        ImGui::Text("%s\n", popup.property->value.c_str());

        for (const auto &entry : popup.propertyValueList) {
            ImGui::BulletText("%.*s\n", static_cast<int>(entry.length()), entry.data());
        }

        ImGui::EndPopup();
    } else {
        popup = {};
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
                scheduleOpenPropertyPopupOnClick(property);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip(property.name.c_str());
                }
                ImGui::TableNextColumn();
                ImGui::Text(property.value.c_str());
                scheduleOpenPropertyPopupOnClick(property);
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

void TargetGraphTab::scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property) {
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        popup.shouldBeOpen = true;
        popup.isOpen = false;
        popup.property = &property;
        popup.propertyValueList = {};

        // If property value contains semicolons, most probably it's a list, because CMake delimits list entries with semicolons.
        // Split the value, so we can display it as a list of bullets in popup.
        size_t currentPosition = 0;
        size_t semicolonPosition = 0;
        while ((semicolonPosition = property.value.find(";", currentPosition)) != std::string::npos) {
            std::string_view entry = std::string_view{property.value}.substr(currentPosition, semicolonPosition - currentPosition);
            popup.propertyValueList.push_back(entry);

            currentPosition = semicolonPosition + 1;
        }
        if (!popup.propertyValueList.empty()) {
            std::string_view entry = std::string_view{property.value}.substr(currentPosition);
            popup.propertyValueList.push_back(entry);
        }
    }
}

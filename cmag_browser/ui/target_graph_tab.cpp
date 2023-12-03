#include "target_graph_tab.h"

#include "cmag_browser/util/imgui_utils.h"
#include "cmag_lib/utils/string_utils.h"

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
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // TODO: extract this to a separate function
    // TODO: do we leave this? Or make sure scales are always correctly calculated?
    ImGui::PushItemWidth(width - ImGui::CalcTextSize("node size").x);
    if (ImGui::SliderFloat("node size", targetGraph.getNodeScalePtr(), 5, 40, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        targetGraph.reinitializeModelMatrices();
    }
    ImGui::PushItemWidth(width - ImGui::CalcTextSize("text size").x);
    if (ImGui::SliderFloat("text size", targetGraph.getTextScalePtr(), 3, 12, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        targetGraph.reinitializeModelMatrices();
    }
    ImGui::PushItemWidth(width - ImGui::CalcTextSize("arrow length").x);
    if (ImGui::SliderFloat("arrow length", targetGraph.getArrowLengthScalePtr(), 1, 15, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        // TODO add refreshConnections()
        targetGraph.reinitializeModelMatrices();
    }
    ImGui::PushItemWidth(width - ImGui::CalcTextSize("arrow width").x);
    if (ImGui::SliderFloat("arrow width", targetGraph.getArrowWidthScalePtr(), 1, 15, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        targetGraph.reinitializeModelMatrices();
    }

    renderPropertyPopup();
    renderPropertyTable(width);
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

                {
                    RaiiImguiStyle style{};
                    if (property.isConsistent) {
                        style.color(ImGuiCol_Text, ImColor::HSV(0.14f, 0.6f, 0.6f));
                    } else {
                        style.color(ImGuiCol_Text, ImColor::HSV(0.5f, 0.6f, 0.6f));
                    }
                    ImGui::Text(property.name.c_str());
                }

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
        targetGraph.update(io);
        targetGraph.render(space.x, space.y);

        // We passed available space to the target graph, but actually used space may be different,
        // because the graph tries to maintain aspect ratio. Hence, we have to query the size before
        // displaying the image.
        const auto texture = (void *)(intptr_t)targetGraph.getTexture();
        const auto textureWidth = static_cast<float>(targetGraph.getTextureWidth());
        const auto textureHeight = static_cast<float>(targetGraph.getTextureHeight());
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (space.x - textureWidth) / 2);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (space.y - textureHeight) / 2);
        ImGui::Image(texture, ImVec2(textureWidth, textureHeight));

        const ImVec2 pos = ImGui::GetItemRectMin();
        targetGraph.savePosition(static_cast<size_t>(pos.x), static_cast<size_t>(pos.y));
    }
}

void TargetGraphTab::scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property) {
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        popup.shouldBeOpen = true;
        popup.isOpen = false;
        popup.property = &property;
        popup.propertyValueList = splitCmakeListString(property.value, true);
    }
}

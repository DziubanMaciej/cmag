#include "target_graph_tab.h"

#include "cmag_browser/components/config_selector.h"
#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/utils/string_utils.h"

TargetGraphTab::TargetGraphTab(CmagBrowserTheme &theme, CmagProject &project, ConfigSelector &configSelector, bool showDebugWidgets)
    : theme(theme),
      targetGraph(theme, project, configSelector.getCurrentConfig()),
      configSelector(configSelector),
      showDebugWidgets(showDebugWidgets) {}

void TargetGraphTab::selectTargetAndFocus(CmagTarget *target) {
    targetGraph.setSelectedTarget(target);
    forceSelection = true;
}

bool TargetGraphTab::fetchForceSelection() {
    const bool result = forceSelection;
    forceSelection = false;
    return result;
}

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
    if (showDebugWidgets) {
        ImGui::Checkbox("Demo Window", &showDemoWindow);
        if (showDemoWindow) {
            ImGui::ShowDemoWindow(&showDemoWindow);
        }

        ImGui::Checkbox("Style selector", &showStyleSelector);
        if (showStyleSelector) {
            if (ImGui::Begin("StyleSelectorWindow")) {
                ImGui::ShowStyleEditor(&ImGui::GetStyle());
                ImGui::End();
            }
        }

        renderSidePaneSlider("node size", width, 5, 40, targetGraph.getNodeScalePtr());
        renderSidePaneSlider("text size", width, 3, 12, targetGraph.getTextScalePtr());
        renderSidePaneSlider("arrow length", width, 1, 15, targetGraph.getArrowLengthScalePtr());
        renderSidePaneSlider("arrow width", width, 1, 15, targetGraph.getArrowWidthScalePtr());
        renderSidePaneSlider("stipple", width, 0.005f, 0.1f, targetGraph.getLineStippleScalePtr());
    }

    configSelector.render(width);
    renderSidePaneDependencyTypeSelection(width);
    if (ImGui::Button("Fit camera")) {
        targetGraph.showEntireGraph();
    }
    if (ImGui::Button("Reset layout")) {
        targetGraph.resetGraphLayout();
        targetGraph.showEntireGraph();
    }
    renderSidePaneHideConnectionsButton(width);
    renderPropertyPopup();
    renderPropertyTable(width);
}

void TargetGraphTab::renderSidePaneSlider(const char *label, float width, float min, float max, float *value) {
    const float textWidth = ImGui::CalcTextSize(label).x;
    std::string labelHidden = std::string("##");

    float sliderWidth = width - textWidth;
    const char *sliderLabel = label;
    bool tooltipNeeded = false;
    const float minimumSliderWidth = 50.f;
    if (textWidth + minimumSliderWidth > width) {
        sliderWidth = width;
        labelHidden.append(label);
        sliderLabel = labelHidden.c_str();
        tooltipNeeded = true;
    }

    ImGui::PushItemWidth(sliderWidth);
    if (ImGui::SliderFloat(sliderLabel, value, min, max, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        targetGraph.refreshModelMatrices();
        targetGraph.refreshConnections();
    }
    if (tooltipNeeded && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", label);
    }
}

void TargetGraphTab::renderSidePaneDependencyTypeSelection(float width) {
    const char *labels[] = {
        "Draw build dependencies",
        "Draw link dependencies",
    };
    const char *tooltips[] = {
        "Derived based on LINK_LIBRARIES (target_link_libraries) and MANUALLY_ADDED_DEPENDENCIES (add_dependencies).",
        "Derived based on LINK_LIBRARIES (target_link_libraries).",
    };
    constexpr int selectionsCount = static_cast<int>(CmakeDependencyType::COUNT);
    static_assert(sizeof(labels) / sizeof(labels[0]) == selectionsCount);
    static_assert(sizeof(tooltips) / sizeof(tooltips[0]) == selectionsCount);

    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##dependencyTypeSelection", labels[dependencyTypeComboSelection])) {
        for (int selectionIndex = 0; selectionIndex < selectionsCount; selectionIndex++) {
            const bool isSelected = (dependencyTypeComboSelection == selectionIndex);
            if (ImGui::Selectable(labels[selectionIndex], isSelected)) {
                dependencyTypeComboSelection = selectionIndex;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", tooltips[selectionIndex]);
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    } else {
        ImGui::SetItemTooltip("%s", tooltips[dependencyTypeComboSelection]);
    }
}

void TargetGraphTab::renderSidePaneHideConnectionsButton(float) {
    CmagTarget *target = targetGraph.getSelectedTarget();
    if (target == nullptr) {
        return;
    }

    if (ImGui::Checkbox("Hide dependencies", &target->graphical.hideConnections)) {
        targetGraph.refreshConnections();
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
    CmagTarget *selectedTarget = targetGraph.getSelectedTarget();
    if (selectedTarget == nullptr) {
        return;
    }

    ImVec2 propertyTableSize = ImGui::GetContentRegionAvail();
    propertyTableSize.x = width;
    const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    auto tableStyle = theme.setupPropertyTable();
    if (ImGui::BeginTable("Table populating", 2, tableFlags, propertyTableSize)) {

        const CmagTargetConfig *config = selectedTarget->tryGetConfig(configSelector.getCurrentConfig());
        for (const CmagTargetProperty &property : config->properties) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            {
                auto nameStyle = theme.setupPropertyName(property.value.empty(), property.isConsistent);
                ImGui::Text("%s", property.name.c_str());
            }
            scheduleOpenPropertyPopupOnClick(property);
            renderPropertyTablePopup(property, false);

            ImGui::TableNextColumn();
            {
                auto valueStyle = theme.setupPropertyValue();
                ImGui::Text("%s", property.value.c_str());
            }
            scheduleOpenPropertyPopupOnClick(property);
            renderPropertyTablePopup(property, true);
        }
    }
    ImGui::EndTable();
}
void TargetGraphTab::renderPropertyTablePopup(const CmagTargetProperty &property, bool showValue) const {
    const char *content = showValue ? property.value.c_str() : property.name.c_str();
    if (Tooltip::begin(theme, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), content)) {
        {
            auto popupStyle = theme.setupPopup();
            auto textStyle = theme.setupPropertyName(false, property.isConsistent);
            const char *contentExtra = property.isConsistent ? "This property has the same value for all configs." : "This property has a different value for different configs.";
            ImGui::Text("%s", contentExtra);
        }
        Tooltip::end();
    }
}

void TargetGraphTab::renderGraph(ImGuiIO &io) {
    ImVec2 space = ImGui::GetContentRegionAvail();

    if (space.x > 0 && space.y > 0) {
        targetGraph.setScreenSpaceAvailableSpace(space.x, space.y);

        // We passed available space to the target graph, but actually used space may be different,
        // because the graph tries to maintain aspect ratio. Hence, we have to query the size.
        const auto textureWidth = static_cast<float>(targetGraph.getTextureWidth());
        const auto textureHeight = static_cast<float>(targetGraph.getTextureHeight());
        const auto textureX = ImGui::GetCursorPosX() + (space.x - textureWidth) / 2;
        const auto textureY = ImGui::GetCursorPosY() + (space.y - textureHeight) / 2;
        targetGraph.setScreenSpacePosition(static_cast<size_t>(textureX), static_cast<size_t>(textureY));

        // Render to an offscreen texture
        targetGraph.setCurrentCmakeConfig(configSelector.getCurrentConfig());
        targetGraph.setDisplayedDependencyType(static_cast<CmakeDependencyType>(dependencyTypeComboSelection));
        targetGraph.update(io);
        targetGraph.render();

        // Display rendered texture
        const auto texture = (void *)(intptr_t)targetGraph.getTexture();
        ImGui::SetCursorPosX(textureX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (space.y - textureHeight) / 2);
        ImGui::Image(texture, ImVec2(textureWidth, textureHeight));

        if (const CmagTarget *target = targetGraph.getFocusedTarget(); target != nullptr) {
            ImGui::SetTooltip("%s\n(%s)", target->name.c_str(), cmagTargetTypeToString(target->type));
        }
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

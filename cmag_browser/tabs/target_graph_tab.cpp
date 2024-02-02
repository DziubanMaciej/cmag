#include "target_graph_tab.h"

#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/utils/string_utils.h"

TargetGraphTab::TargetGraphTab(BrowserState &browser, bool showDebugWidgets)
    : browser(browser),
      targetGraph(browser),
      showDebugWidgets(showDebugWidgets) {}

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

    browser.getConfigSelector().render(width);
    browser.getConfigSelector().renderTooltipLastItem();
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
    if (tooltipNeeded) {
        TooltipBuilder(browser.getTheme())
            .setHoverLastItem()
            .addText(label)
            .execute();
    }
}

void TargetGraphTab::renderSidePaneDependencyTypeSelection(float width) {
    const char *labels[] = {
        "Build dependencies",
        "Interface dependencies",
        "Manual dependencies",
    };
    const char *tooltips[] = {
        "Show dependencies derived based on LINK_LIBRARIES. This property is usually populated with target_link_libraries() with PUBLIC or PRIVATE argument.",
        "Show dependencies derived based on INTERFACE_LINK_LIBRARIES. This property is usually populated with target_link_libraries() with PUBLIC or INTERFACE argument.",
        "Show dependencies derived based on MANUALLY_ADDED_DEPENDENCIES. This property is usually populated with add_dependencies().",
    };
    constexpr int selectionsCount = static_cast<int>(CmakeDependencyType::COUNT);
    static_assert(sizeof(labels) / sizeof(labels[0]) == selectionsCount);
    static_assert(sizeof(tooltips) / sizeof(tooltips[0]) == selectionsCount);

    auto renderCheckbox = [&](CmakeDependencyType currentType, int typeIndex) {
        FATAL_ERROR_IF(typeIndex >= selectionsCount, "Incorrect type index");

        bool isSelected = (dependencyTypeSelected & currentType) != CmakeDependencyType::NONE;
        ImGui::SetNextItemWidth(width);
        if (ImGui::Checkbox(labels[typeIndex], &isSelected)) {
            dependencyTypeSelected = dependencyTypeSelected ^ currentType;
        }
        TooltipBuilder(browser.getTheme())
            .setHoverLastItem()
            .addText(tooltips[typeIndex])
            .execute();
    };

    renderCheckbox(CmakeDependencyType::Build, 0);
    renderCheckbox(CmakeDependencyType::Interface, 1);
    renderCheckbox(CmakeDependencyType::Additional, 2);
}

void TargetGraphTab::renderSidePaneHideConnectionsButton(float) {
    CmagTarget *target = browser.getTargetSelection().getMutableSelection();
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
    const CmagBrowserTheme &theme = browser.getTheme();

    const CmagTarget *selectedTarget = browser.getTargetSelection().getSelection();
    if (selectedTarget == nullptr) {
        return;
    }

    const ImVec2 propertyTableSize{width, 0};
    const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    auto tableStyle = theme.setupPropertyTable();
    if (ImGui::BeginTable("Table populating", 2, tableFlags, propertyTableSize)) {

        const CmagTargetConfig *config = selectedTarget->tryGetConfig(browser.getConfigSelector().getCurrentConfig());
        for (const CmagTargetProperty &property : config->properties) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            {
                renderPropertyTablePopup(property, false);

                auto nameStyle = theme.setupPropertyName(property.value.empty(), property.isConsistent);
                ImGui::Text("%s", property.name.c_str());

                scheduleOpenPropertyPopupOnClick(property);
            }

            ImGui::TableNextColumn();
            {
                renderPropertyTablePopup(property, true);

                auto valueStyle = theme.setupPropertyValue();
                ImGui::Text("%s", property.value.c_str());

                scheduleOpenPropertyPopupOnClick(property);
            }
        }
    }
    ImGui::EndTable();
}
void TargetGraphTab::renderPropertyTablePopup(const CmagTargetProperty &property, bool showValue) const {
    const CmagBrowserTheme &theme = browser.getTheme();

    const ImVec2 cellMin = ImGui::GetCursorPos();
    const ImVec2 cellMax = {cellMin.x + ImGui::GetContentRegionAvail().x, cellMin.y + ImGui::CalcTextSize("").y};

    TooltipBuilder(theme)
        .setHoverRect(cellMin, cellMax)
        .addTextOneLine(showValue ? property.value.c_str() : property.name.c_str())
        .execute([&]() {
            auto popupStyle = theme.setupPopup();
            auto textStyle = theme.setupPropertyName(false, property.isConsistent);
            const char *contentExtra = property.isConsistent ? "This property has the same value for all configs." : "This property has a different value for different configs.";
            ImGui::Text("%s", contentExtra);
        });
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
        targetGraph.setCurrentCmakeConfig(browser.getConfigSelector().getCurrentConfig());
        targetGraph.setDisplayedDependencyType(dependencyTypeSelected);
        targetGraph.update(io);
        targetGraph.render();

        // Display rendered texture
        const auto texture = (void *)(intptr_t)targetGraph.getTexture();
        ImGui::SetCursorPosX(textureX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (space.y - textureHeight) / 2);
        ImGui::Image(texture, ImVec2(textureWidth, textureHeight));

        // Display popups
        if (CmagTarget *target = targetGraph.getFocusedTarget(); target != nullptr) {
            renderTargetPopup(io, target);
        }
        if (TargetGraph::ConnectionData *connection = targetGraph.getFocusedConnection(); connection != nullptr) {
            renderConnectionPopup(connection);
        }
    }
}
void TargetGraphTab::renderTargetPopup(const ImGuiIO &io, CmagTarget *target) {
    if (io.MouseClicked[ImGuiMouseButton_Right]) {
        browser.getTabChange().showPopup(TabChange::TargetGraph, target);
    }
    if (browser.getTabChange().isPopupShown()) {
        return;
    }

    std::string text = target->name + " (" + cmagTargetTypeToString(target->type) + ")";
    TooltipBuilder(browser.getTheme())
        .setHoverAlways()
        .addText(text.c_str())
        .execute();
}
void TargetGraphTab::renderConnectionPopup(const TargetGraph::ConnectionData *connection) {
    std::string text = connection->src->name + " -> " + connection->dst->name;

    const char *dependencyTypeText = "Unknown dependency";
    switch (connection->type) {
    case CmakeDependencyType::Build:
        dependencyTypeText = "Build dependency";
        break;
    case CmakeDependencyType::Interface:
        dependencyTypeText = "Interface dependency";
        break;
    case CmakeDependencyType::Additional:
        dependencyTypeText = "Manual dependency";
        break;
    default:
        LOG_WARNING("Unknown dependency type");
    }

    TooltipBuilder(browser.getTheme())
        .setHoverAlways()
        .addText(text.c_str())
        .addText(dependencyTypeText)
        .execute();
}

void TargetGraphTab::scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property) {
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        popup.shouldBeOpen = true;
        popup.isOpen = false;
        popup.property = &property;
        popup.propertyValueList = splitCmakeListString(property.value, true);
    }
}

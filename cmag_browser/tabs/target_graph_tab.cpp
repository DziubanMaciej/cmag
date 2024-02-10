#include "target_graph_tab.h"

#include "cmag_browser/browser_state/cmag_browser_theme.h"
#include "cmag_browser/ui_utils/section.h"
#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/utils/string_utils.h"

TargetGraphTab::TargetGraphTab(BrowserState &browser, bool showDebugWidgets)
    : browser(browser),
      targetGraph(browser),
      showDebugWidgets(showDebugWidgets) {}

void TargetGraphTab::render(ImGuiIO &io) {
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float sidePaneWidth = windowWidth * 0.2f;

    renderSidePane(sidePaneWidth);
    ImGui::SameLine();
    renderGraph(io);
}

void TargetGraphTab::renderSidePane(float width) {
    const ImVec2 sidePaneSize = {width, ImGui::GetContentRegionAvail().y};
    if (ImGui::BeginChild("TargetGraphTabSidePane", sidePaneSize)) {
        renderSidePaneSectionDebug();
        renderSidePaneSectionView();
        renderSidePaneSectionTarget();
    }
    ImGui::EndChild();
}

void TargetGraphTab::renderSidePaneSectionDebug() {
    if (!showDebugWidgets) {
        return;
    }

    Section section{"Debug", sectionIndentSize, marginBetweenSections};

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

    renderSidePaneSlider("node size", 5, 40, targetGraph.getNodeScalePtr());
    renderSidePaneSlider("text size", 3, 12, targetGraph.getTextScalePtr());
    renderSidePaneSlider("arrow length", 1, 15, targetGraph.getArrowLengthScalePtr());
    renderSidePaneSlider("arrow width", 1, 15, targetGraph.getArrowWidthScalePtr());
    renderSidePaneSlider("stipple", 0.005f, 0.1f, targetGraph.getLineStippleScalePtr());

    browser.getConfigSelector().render(ImGui::GetContentRegionAvail().x);
    browser.getConfigSelector().renderTooltipLastItem();
}

void TargetGraphTab::renderSidePaneSectionView() {
    Section section{"View", sectionIndentSize, marginBetweenSections};

    renderSidePaneDependencyTypeSelection();

    const ImVec2 buttonSize = ImVec2{ImGui::GetContentRegionAvail().x, 0};
    if (ImGui::Button("Fit camera", buttonSize)) {
        targetGraph.showEntireGraph();
    }
    TooltipBuilder(browser.getTheme())
        .setHoverLastItem()
        .addText("Set camera position and zoom automatically, so that all targets are visible.")
        .execute();

    if (ImGui::Button("Reset layout", buttonSize)) {
        targetGraph.resetGraphLayout();
        targetGraph.showEntireGraph();
    }
    TooltipBuilder(browser.getTheme())
        .setHoverLastItem()
        .addText("Recalculate positions of targets on the graph.")
        .execute();
}

void TargetGraphTab::renderSidePaneSectionTarget() {
    CmagTarget *target = browser.getTargetSelection().getMutableSelection();
    if (target == nullptr) {
        return;
    }

    const std::string label = "Target " + target->name;
    Section section{label.c_str(), sectionIndentSize, marginBetweenSections};

    if (ImGui::Checkbox("Hide dependencies", &target->graphical.hideConnections)) {
        targetGraph.refreshConnections();
    }
    TooltipBuilder(browser.getTheme())
        .setHoverLastItem()
        .addText("Hide all connections between this target and other targets on graph. Useful to avoid clutter.")
        .execute();

    renderPropertyPopup();
    renderPropertyTable(target);
}

void TargetGraphTab::renderSidePaneSlider(const char *label, float min, float max, float *value) {
    const float textWidth = ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(label).x;
    std::string labelHidden = std::string("##");

    const float sidePaneWidth = ImGui::GetContentRegionAvail().x;
    float sliderWidth = sidePaneWidth - textWidth;
    const char *sliderLabel = label;
    bool tooltipNeeded = false;
    const float minimumSliderWidth = 50.f;
    if (textWidth + minimumSliderWidth > sidePaneWidth) {
        sliderWidth = sidePaneWidth;
        labelHidden.append(label);
        sliderLabel = labelHidden.c_str();
        tooltipNeeded = true;
    }

    ImGui::PushItemWidth(sliderWidth); // TODO integrate this in RaiiImguiStyle
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
    ImGui::PopItemWidth();
}

void TargetGraphTab::renderSidePaneDependencyTypeSelection() {
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
    constexpr int selectionsCount = static_cast<int>(CmagDependencyType::COUNT);
    static_assert(sizeof(labels) / sizeof(labels[0]) == selectionsCount);
    static_assert(sizeof(tooltips) / sizeof(tooltips[0]) == selectionsCount);

    CmagDependencyType &dependencyTypeSelected = browser.getProject().getGlobals().browser.displayedDependencyType;

    auto renderCheckbox = [&](CmagDependencyType currentType, int typeIndex) {
        FATAL_ERROR_IF(typeIndex >= selectionsCount, "Incorrect type index");

        bool isSelected = (dependencyTypeSelected & currentType) != CmagDependencyType::NONE;
        if (ImGui::Checkbox(labels[typeIndex], &isSelected)) {
            dependencyTypeSelected = dependencyTypeSelected ^ currentType;
            browser.getProjectSaver().makeDirty(ProjectDirtyFlag::SelectedDependencies);
        }
        TooltipBuilder(browser.getTheme())
            .setHoverLastItem()
            .addText(tooltips[typeIndex])
            .execute();
    };

    renderCheckbox(CmagDependencyType::Build, 0);
    renderCheckbox(CmagDependencyType::Interface, 1);
    renderCheckbox(CmagDependencyType::Additional, 2);
}

void TargetGraphTab::renderPropertyPopup() {
    constexpr const char *popupName = "propertyPopup";

    if (popup.shouldBeOpen && !popup.isOpen) {
        ImGui::OpenPopup(popupName);
        popup.isOpen = true;
    }

    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(0, 0),
        ImVec2(displaySize.x / 2, displaySize.y / 2));
    if (ImGui::BeginPopup(popupName)) {
        ImGui::Text("Property %s", popup.property->name.c_str());

        char buffer[1024];
        for (const auto &entry : popup.propertyValueList) {
            snprintf(buffer, sizeof(buffer), "  %.*s\n", static_cast<int>(entry.length()), entry.data());
            ImGui::Selectable(buffer, false);
        }

        ImGui::EndPopup();
    } else {
        popup = {};
    }
}

void TargetGraphTab::renderPropertyTable(const CmagTarget *selectedTarget) {
    const CmagBrowserTheme &theme = browser.getTheme();

    const ImVec2 propertyTableSize{ImGui::GetContentRegionAvail().x - 1.0f, 0}; // Not sure why -1 is needed, but without it the right border is clipped...
    const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    auto tableStyle = theme.setupPropertyTable();
    if (ImGui::BeginTable("Table populating", 2, tableFlags, propertyTableSize)) {
        const CmagTargetConfig *config = selectedTarget->tryGetConfig(browser.getConfigSelector().getCurrentConfig());

        for (const CmagTargetProperty &property : config->properties) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            {
                const ImVec2 cellMin = ImGui::GetCursorScreenPos();
                const ImVec2 cellMax = {cellMin.x + ImGui::GetContentRegionAvail().x, cellMin.y + ImGui::CalcTextSize("").y};

                renderPropertyTablePopup(property, cellMin, cellMax, false);
                scheduleOpenPropertyPopupOnClick(property, cellMin, cellMax);

                auto nameStyle = theme.setupPropertyName(property.value.empty(), property.isConsistent);
                ImGui::Text("%s", property.name.c_str());
            }

            ImGui::TableNextColumn();
            {
                const ImVec2 cellMin = ImGui::GetCursorScreenPos();
                const ImVec2 cellMax = {cellMin.x + ImGui::GetContentRegionAvail().x, cellMin.y + ImGui::CalcTextSize("").y};

                renderPropertyTablePopup(property, cellMin, cellMax, true);
                scheduleOpenPropertyPopupOnClick(property, cellMin, cellMax);

                auto valueStyle = theme.setupPropertyValue();
                ImGui::Text("%s", property.value.c_str());
            }
        }

        ImGui::EndTable();
    }
}

void TargetGraphTab::renderPropertyTablePopup(const CmagTargetProperty &property, ImVec2 cellMin, ImVec2 cellMax, bool showValue) const {
    const CmagBrowserTheme &theme = browser.getTheme();
    TooltipBuilder(theme)
        .setHoverRect(cellMin, cellMax)
        .addTextOneLine(showValue ? property.value.c_str() : property.name.c_str())
        .hideWhenPopupsAreVisible()
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
        targetGraph.setDisplayedDependencyType(browser.getProject().getGlobals().browser.displayedDependencyType);
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
    case CmagDependencyType::Build:
        dependencyTypeText = "Build dependency";
        break;
    case CmagDependencyType::Interface:
        dependencyTypeText = "Interface dependency";
        break;
    case CmagDependencyType::Additional:
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

void TargetGraphTab::scheduleOpenPropertyPopupOnClick(const CmagTargetProperty &property, ImVec2 cellMin, ImVec2 cellMax) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(cellMin, cellMax)) {
        popup.shouldBeOpen = true;
        popup.isOpen = false;
        popup.property = &property;
        popup.propertyValueList = splitCmakeListString(property.value, true);
        if (popup.propertyValueList.empty() && !popup.property->value.empty()) {
            popup.propertyValueList.push_back(popup.property->value);
        }
    }
}

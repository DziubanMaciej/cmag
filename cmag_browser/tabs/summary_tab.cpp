#include "summary_tab.h"

#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/core/cmag_project.h"

SummaryTab::SummaryTab(BrowserState &browser)
    : browser(browser),
      compiler(createCompilerString(browser.getProject().getGlobals())) {
}

void SummaryTab::render() {
    const CmagGlobals &globals = browser.getProject().getGlobals();

    renderSectionHeader("Environment", "Details of environment in which cmag dumper worked to dump this project. Note that this may be different than your current environment.");
    renderRowString("Source directory", globals.sourceDir, "Directory with source files of the generated CMake project at a time of generation. Note that this path could be no longer available on your current system.");
    renderRowString("Build directory", globals.buildDir, "Build directory of the generate CMake project. Note that this path could be no longer available on your current system.");
    renderRowString("Compiler", compiler, "Compiler used for the CMake project. It usually depends on CMake generator and/or CC and CXX environment variables.");
    renderRowString("Operating system", globals.os, "Operating system on which the project was generated.");

    renderSectionHeader("CMake configuration", "Metadata about dumped CMake build-system.");
    renderRowString("CMake version", globals.cmakeVersion, "Version of CMake used to generate this project. This may differ from currently installed cmake version.");
    renderRowConfigSelector();
    renderRowString("CMake project name", globals.cmakeProjectName, "Name of the CMake project set by project() command in the root level CMakeLists.txt. More info at:", "https://cmake.org/cmake/help/latest/command/project.html");
    renderRowString("CMake generator", globals.generator, "CMake generation used to generate this project. Generators can be thought of as a compiler/build system selectors. More info at:", "https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html");

    renderSectionHeader("Cmag configuration", "Metadata cmag dumper that created this project.");
    renderRowString("Cmag version", globals.cmagVersion.toString(), "Version of cmag that was used to generate this project.");
    renderRowString("Cmag project name", globals.cmagProjectName, "Name of the cmag project set with -p argument to cmag. This name doesn't have any real implications, though it can be used to differentiate between cmag project files.");

    renderSectionHeader("Saving", "Settings for saving changes to current project, like positions of nodes in the graph or currently viewed tab.");
    renderRowSave();
    renderRowAutoSave();
}

void SummaryTab::renderSectionHeader(const char *name, const char *tooltip) {
    ImGui::Text(" ");
    ImGui::SeparatorText(name);
    TooltipBuilder(browser.getTheme())
        .setHoverLastItem()
        .addText(tooltip)
        .execute();
}

void SummaryTab::renderRowLabel(const char *name) {
    constexpr static float additionalPadding = 75.f;
    const float size = ImGui::CalcTextSize(name).x + additionalPadding;
    firstColumnWidth = std::max(firstColumnWidth, size);

    const ImVec2 posMin = ImGui::GetCursorPos();
    ImGui::Text("%s", name);
    ImGui::SameLine();
    ImGui::SetCursorPosX(posMin.x + firstColumnWidth);
}

void SummaryTab::renderRowString(const char *name, const std::string &value, const char *tooltip, const char *tooltipHyperlink) {
    // Label
    const ImVec2 posMin = ImGui::GetCursorPos();
    renderRowLabel(name);

    // Value
    ImGui::Text("%s", value.c_str());

    // Tooltip
    const ImVec2 posMax = ImGui::GetItemRectMax();
    TooltipBuilder(browser.getTheme())
        .addHyperlink(tooltipHyperlink)
        .setHoverRect(posMin, posMax)
        .addText(tooltip)
        .execute();
}
void SummaryTab::renderRowConfigSelector() {
    // Label
    const ImVec2 posMin = ImGui::GetCursorPos();
    renderRowLabel("Selected config");

    // Value
    browser.getConfigSelector().render(0);

    // Tooltip
    const ImVec2 posMax = ImGui::GetItemRectMax();
    browser.getConfigSelector().renderTooltipRect(posMin, posMax);
}

void SummaryTab::renderRowSave() {
    // Label
    const ImVec2 posMin = ImGui::GetCursorPos();
    renderRowLabel("Manual save");

    // Value
    ProjectSaver &saver = browser.getProjectSaver();
    const bool disableSaveButton = !saver.shouldShowDirtyNotification();
    if (disableSaveButton) {
        ImGui::BeginDisabled();
    }
    const std::string buttonText = std::string("Save ") + saver.getOutputPath().filename().string();
    if (ImGui::Button(buttonText.c_str())) {
        browser.getProjectSaver().save();
    }
    if (disableSaveButton) {
        ImGui::EndDisabled();
    }

    // Tooltip
    const ImVec2 posMax = ImGui::GetItemRectMax();
    TooltipBuilder(browser.getTheme())
        .setHoverRect(posMin, posMax)
        .addText("Ctrl+S")
        .execute();
}

void SummaryTab::renderRowAutoSave() {
    // Label
    const ImVec2 posMin = ImGui::GetCursorPos();
    renderRowLabel("Enable autosave");

    // Value
    bool &autoSaveEnabled = browser.getProject().getGlobals().browser.autoSaveEnabled;
    if (ImGui::Checkbox("##Autosave", &autoSaveEnabled)) {
        browser.getProjectSaver().save();
    }

    // Tooltip
    const ImVec2 posMax = ImGui::GetItemRectMax();
    TooltipBuilder(browser.getTheme())
        .setHoverRect(posMin, posMax)
        .addText("If this option is enabled, the project file will be automatically saved whenever you make any changes.")
        .execute();
}

std::string SummaryTab::createCompilerString(const CmagGlobals &globals) {
    return globals.compilerId + " " + globals.compilerVersion;
}

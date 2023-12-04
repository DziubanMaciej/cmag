#include "summary_tab.h"

#include "cmag_browser/ui/config_selector.h"
#include "cmag_browser/ui_utils/raii_imgui_style.h"
#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_lib/core/cmag_project.h"

SummaryTab::SummaryTab(const CmagBrowserTheme &theme, CmagProject &project, ConfigSelector &configSelector)
    : theme(theme),
      project(project),
      configSelector(configSelector),
      compiler(project.getGlobals().compilerId + " " + project.getGlobals().compilerVersion) {
}

void SummaryTab::render() {
    const CmagGlobals &globals = project.getGlobals();

    const ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("Table populating", 2, tableFlags)) {
        renderTableRowSelectedConfig();
        renderTableRowSpacer();

        renderTableRowString("CMake version", globals.cmakeVersion, "Version of CMake used to generate this project. This may differ from currently installed cmake version.");
        renderTableRowString("CMake project name", globals.cmakeProjectName, "Name of the CMake project set by project() command in the root level CMakeLists.txt. More info at:", "https://cmake.org/cmake/help/latest/command/project.html");
        renderTableRowString("CMake generator", globals.generator, "CMake generation used to generate this project. Generators can be thought of as a compiler/build system selectors. More info at:", "https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html");
        renderTableRowSpacer();

        renderTableRowString("Cmag version", globals.cmagVersion, "Version of cmag that was used to generate this project.");
        renderTableRowString("Cmag project name", globals.cmagProjectName, "Name of the cmag project set with -p argument to cmag. This name doesn't have any real implications, though it can be used to differentiate between cmag project files.");
        renderTableRowSpacer();

        renderTableRowString("Source directory", globals.sourceDir, "Directory with source files of the generated CMake project at a time of generation. Note that this path could be no longer available on your current system.");
        renderTableRowString("Build directory", globals.buildDir, "Build directory of the generate CMake project. Note that this path could be no longer available on your current system.");
        renderTableRowString("Compiler", compiler, "Compiler used for the CMake project. It usually depends on CMake generator and/or CC and CXX environment variables.");
        renderTableRowString("Operating system", globals.os, "Operating system on which the project was generated.");

        ImGui::EndTable();
    }
}

void SummaryTab::renderTableRowString(const char *name, const std::string &value, const char *tooltip, const char *tooltipHyperlink) {

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    const ImVec2 posMin = ImGui::GetCursorPos();
    ImGui::Text("%s", name);
    ImGui::TableNextColumn();
    ImGui::Text("%s", value.c_str());
    const ImVec2 posMax = ImGui::GetItemRectMax();

    Tooltip::renderTooltip(theme, posMin, posMax, tooltip, tooltipHyperlink);
}
void SummaryTab::renderTableRowSelectedConfig() {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    const ImVec2 posMin = ImGui::GetCursorPos();
    ImGui::Text("Selected config");
    ImGui::TableNextColumn();
    configSelector.render(0, true);
    const ImVec2 posMax = ImGui::GetItemRectMax();

    if (Tooltip::isRectHovered(posMin, posMax)) {
        configSelector.renderTooltip();
    }
}

void SummaryTab::renderTableRowSpacer() {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text(" ");
}

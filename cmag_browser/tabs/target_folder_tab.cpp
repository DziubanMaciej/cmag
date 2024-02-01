#include "target_folder_tab.h"

#include "cmag_browser/tabs/target_graph_tab.h"
#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_browser/ui_utils/raii_imgui_style.h"
#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/core/cmake_generator.h"
#include "cmag_core/utils/string_utils.h"

TargetFolderTab::TargetFolderTab(const CmagBrowserTheme &theme, CmagProject &project, TargetGraphTab &targetGraphTab)
    : theme(theme),
      project(project),
      targetGraphTab(targetGraphTab) {}

void TargetFolderTab::render() {
    renderHeaders();
    renderFolder(false, project.getGlobals().derived.folders[0]);
}

void TargetFolderTab::renderHeaders() {
    renderHeadersWarnings();
    ImGui::Checkbox("Show ignored targets", &showIgnoredTargets);
}

void TargetFolderTab::renderHeadersWarnings() {
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const CmagGlobals &globals = project.getGlobals();

    RaiiImguiStyle style{};
    style.textWrapWidth(windowWidth);
    style.color(ImGuiCol_Text, theme.colorWarning);

    const CMakeGenerator *generator = globals.derived.generator;
    if (globals.derived.folders[0].childIndices.empty()) {
        ImGui::Text("The project does not use FOLDER property for its targets, so there is nothing interesting to be shown in this tab.");
        return;
    }

    if (generator && !generator->supportsTargetFolders) {
        ImGui::Text("Warning: the %s generator does not visualize FOLDER property in any way, so this structure cannot be seen in any tool.", generator->name.c_str());
        return;
    }

    if (isCMakeFalse(globals.useFolders)) {
        ImGui::Text("Warning: the property USE_FOLDERS is set to %s, which will disable visualization of FOLDER property in your IDE.", globals.useFolders.c_str());
    } else if (!isCMakeTrue(globals.useFolders)) {
        ImGui::Text("Warning: the property USE_FOLDERS is not set to a valid boolean value. Visualization of FOLDER property in your IDE may or may not work depending on your CMake version and policy CMP0143");
    }
}

void TargetFolderTab::renderFolder(bool renderSelf, const CmagFolder &folder) {
    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
    if (folder.childIndices.empty() && folder.targetIndices.empty()) {
        treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool renderChildren = true;
    if (renderSelf) {
        renderChildren = ImGui::TreeNodeEx(folder.fullName.c_str(), treeNodeFlags, u8"\u01A4 %s", folder.fullName.c_str());
    }

    if (renderChildren) {
        for (const size_t childFolderIndex : folder.childIndices) {
            const CmagFolder &childFolder = project.getGlobals().derived.folders[childFolderIndex];
            renderFolder(true, childFolder);
        }

        for (const size_t targetIndex : folder.targetIndices) {
            CmagTarget &target = project.getTargets()[targetIndex];
            if (showIgnoredTargets || !target.isIgnoredImportedTarget()) {
                renderTarget(target);
            }
        }

        if (renderSelf) {
            ImGui::TreePop();
        }
    }
}

void TargetFolderTab::renderTarget(CmagTarget &target) {
    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Leaf;
    const char *targetName = target.name.c_str();
    const char *targetType = cmagTargetTypeToString(target.type);
    const bool treeNodeOpen = ImGui::TreeNodeEx(targetName, treeNodeFlags, u8"\u01A5 %s (%s)", targetName, targetType);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        targetGraphTab.selectTargetAndFocus(&target);
    }

    if (treeNodeOpen) {
        ImGui::TreePop();
    }
}
#include "target_folder_tab.h"

#include "cmag_browser/components/target_graph_tab.h"
#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_browser/ui_utils/raii_imgui_style.h"
#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/core/cmake_generator.h"

TargetFolderTab::TargetFolderTab(const CmagBrowserTheme &theme, CmagProject &project, TargetGraphTab &targetGraphTab)
    : theme(theme),
      project(project),
      targetGraphTab(targetGraphTab) {}

void TargetFolderTab::render() {
    renderOptions();
    renderFolder(false, project.getGlobals().derived.folders[0]);
}

void TargetFolderTab::renderOptions() {
    const float windowWidth = ImGui::GetContentRegionAvail().x;

    const CMakeGenerator *generator = project.getGlobals().derived.generator;
    if (project.getGlobals().derived.folders[0].childIndices.empty()) {
        RaiiImguiStyle style{};
        style.textWrapWidth(windowWidth);
        style.color(ImGuiCol_Text, theme.colorWarning);
        ImGui::Text("The project does not use FOLDER property for its targets, so there is nothing interesting to be shown in this tab.");
    } else if (generator && !generator->supportsTargetFolders) {
        RaiiImguiStyle style{};
        style.textWrapWidth(windowWidth);
        style.color(ImGuiCol_Text, theme.colorWarning);
        ImGui::Text("Warning: the %s generator does not visualize FOLDER property in any way, so this structure cannot be seen in any tool.", generator->name.c_str());
    }

    ImGui::Checkbox("Show ignored targets", &showIgnoredTargets);
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

#include "target_folder_tab.h"

#include "cmag_browser/tabs/target_graph_tab.h"
#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_core/core/cmake_generator.h"
#include "cmag_core/utils/string_utils.h"

TargetFolderTab::TargetFolderTab(BrowserState &browser)
    : browser(browser) {}

void TargetFolderTab::render() {
    renderHeaders();
    renderFolder(false, browser.getProject().getGlobals().derived.folders[0]);
}

void TargetFolderTab::renderHeaders() {
    renderHeadersWarnings();
    ImGui::Checkbox("Show ignored targets", &showIgnoredTargets);
}

void TargetFolderTab::renderHeadersWarnings() {
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const CmagGlobals &globals = browser.getProject().getGlobals();

    RaiiImguiStyle style{};
    style.textWrapWidth(windowWidth);
    style.color(ImGuiCol_Text, browser.getTheme().colorWarning);

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
            const CmagFolder &childFolder = browser.getProject().getGlobals().derived.folders[childFolderIndex];
            renderFolder(true, childFolder);
        }

        for (const size_t targetIndex : folder.targetIndices) {
            CmagTarget &target = browser.getProject().getTargets()[targetIndex];
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
    const char *targetName = target.name.c_str();
    const char *targetType = cmagTargetTypeToString(target.type);

    // TODO make this more robust
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), u8"\u01A5 %s (%s)", targetName, targetType);

    ImGui::Selectable(buffer, browser.getTargetSelection().isSelected(target));

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        browser.getTargetSelection().select(&target);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        browser.getTargetSelection().select(&target);
        browser.getTabChange().change(TabChange::TargetGraph);
    }
}

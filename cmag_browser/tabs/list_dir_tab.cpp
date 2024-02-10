#include "list_dir_tab.h"

#include "cmag_browser/tabs/target_graph_tab.h"
#include "cmag_browser/ui_utils/tooltip.h"

ListDirTab::ListDirTab(BrowserState &browser)
    : browser(browser) {}

void ListDirTab::render() {
    renderOptions();
    renderListDir(nullptr, browser.getProject().getGlobals().listDirs[0]);
}

void ListDirTab::renderOptions() {
    ImGui::Checkbox("Show ignored targets", &showIgnoredTargets);
    TooltipBuilder(browser.getTheme())
        .setHoverLastItem()
        .addText("Some targets are automatically ignored by cmag due to their lack of use.")
        .execute();
}

void ListDirTab::renderListDir(const char *parentName, const CmagListDir &listDir) {
    const char *relativeName = deriveRelativeName(parentName, listDir.name);

    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
    if (listDir.childIndices.empty() && listDir.derived.targetIndices.empty()) {
        treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool treeNodeOpen = ImGui::TreeNodeEx(listDir.name.c_str(), treeNodeFlags, u8"\u01A4 %s", relativeName);
    renderTooltip(listDir.name);
    if (treeNodeOpen) {
        for (const size_t listDirIndex : listDir.childIndices) {
            const CmagListDir &childListDir = browser.getProject().getGlobals().listDirs[listDirIndex];
            renderListDir(listDir.name.c_str(), childListDir);
        }

        for (const size_t targetIndex : listDir.derived.targetIndices) {
            CmagTarget &target = browser.getProject().getTargets()[targetIndex];
            if (showIgnoredTargets || !target.isIgnoredImportedTarget()) {
                renderTarget(target);
            }
        }

        ImGui::TreePop();
    }
}

void ListDirTab::renderTarget(CmagTarget &target) {
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
        browser.getTabChange().showPopup(TabChange::TabSelection::ListDir, &target);
    }
}

void ListDirTab::renderTooltip(const std::string &currentName) {
    TooltipBuilder(browser.getTheme())
        .setHoverLastItem()
        .addTextOneLine(currentName.c_str())
        .execute();
}

const char *ListDirTab::deriveRelativeName(const char *parentName, const std::string &currentName) {
    if (parentName == nullptr) {
        return "root";
    }

    const size_t pos = currentName.find(parentName);
    if (pos != 0) {
        // Not sure if this is even possible, but let's give up instead of throwing and error.
        return currentName.c_str();
    }

    return currentName.c_str() + strlen(parentName) + 1;
}

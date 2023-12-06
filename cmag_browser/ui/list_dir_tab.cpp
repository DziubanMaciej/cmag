#include "list_dir_tab.h"

#include "cmag_browser/ui_utils/tooltip.h"
#include "cmag_lib/core/cmag_project.h"

ListDirTab::ListDirTab(const CmagBrowserTheme &theme, const CmagProject &project)
    : theme(theme),
      project(project) {}

void ListDirTab::render() {
    renderListDir(nullptr, project.getGlobals().listDirs[0]);
}

void ListDirTab::renderListDir(const char *parentName, const CmagListDir &listDir) {
    const char *relativeName = deriveRelativeName(parentName, listDir.name);

    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
    if (listDir.childIndices.empty() && listDir.derived.targetIndices.empty()) {
        treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool treeNodeOpen = ImGui::TreeNodeEx(listDir.name.c_str(), treeNodeFlags, "%s", relativeName);
    renderTooltip(listDir.name);
    if (treeNodeOpen) {
        for (const size_t listDirIndex : listDir.childIndices) {
            const CmagListDir &childListDir = project.getGlobals().listDirs[listDirIndex];
            renderListDir(listDir.name.c_str(), childListDir);
        }

        for (const size_t targetIndex : listDir.derived.targetIndices) {
            const CmagTarget &target = project.getTargets()[targetIndex];
            renderTarget(target);
        }

        ImGui::TreePop();
    }
}

void ListDirTab::renderTarget(const CmagTarget &target) {
    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Leaf;
    if (ImGui::TreeNodeEx(target.name.c_str(), treeNodeFlags, "%s", target.name.c_str())) {
        ImGui::TreePop();
    }
}

void ListDirTab::renderTooltip(const std::string &currentName) {
    if (Tooltip::begin(theme, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), currentName.c_str(), nullptr, true)) {
        Tooltip::end();
    }
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

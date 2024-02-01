#include "cmag_browser/tab_change.h"

#include <imgui.h>

void TabChange::change(TabSelection newSelection) {
    if (selection != TabSelection::Auto) {
        LOG_WARNING("Multiple tab selection in one frame detected. Only last one will be honored");
    }

    selection = newSelection;
}

TabChange::TabSelection TabChange::fetch() {
    const TabSelection result = selection;
    selection = TabSelection::Auto;
    return result;
}

void TabChange::renderPopup() {
    constexpr const char *popupName = "tabChangePopup";

    const static TabSelection allTabs[] = {
        TargetGraph,
        ListDir,
        TargetFolder,
    };
    const static char *tabNames[] = {
        nullptr,
        "Show on target graph",
        "Show in list files",
        "Show in target folders",
    };

    if (popup.shouldBeOpen && !popup.isOpen) {
        ImGui::OpenPopup(popupName);
        popup.isOpen = true;
    }

    if (ImGui::BeginPopup(popupName)) {
        for (TabSelection tabSelection : allTabs) {
            if (tabSelection == popup.currentTab) {
                continue;
            }

            const char *tabName = tabNames[static_cast<int>(tabSelection)];
            if (ImGui::Selectable(tabName, false)) {
                popup = {};
                this->change(tabSelection);
            }
        }

        ImGui::EndPopup();
    } else {
        popup = {};
    }
}
void TabChange::showPopup(TabChange::TabSelection currentTab) {
    popup.shouldBeOpen = true;
    popup.isOpen = false;
    popup.currentTab = currentTab;
}

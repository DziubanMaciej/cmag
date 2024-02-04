#include "cmag_browser/browser_state/browser_state.h"
#include "cmag_browser/browser_state/tab_change.h"

#include <imgui.h>

TabChange::TabChange(BrowserState &browser)
    : browser(browser) {}

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
                ImGui::BeginDisabled();
            }

            const char *tabName = tabNames[static_cast<int>(tabSelection)];
            if (ImGui::Selectable(tabName, false)) {
                this->change(tabSelection);
                browser.getTargetSelection().select(popup.targetToSelect);
                popup = {};
            }

            if (tabSelection == popup.currentTab) {
                ImGui::EndDisabled();
            }
        }

        ImGui::EndPopup();
    } else {
        popup = {};
    }
}
void TabChange::showPopup(TabChange::TabSelection currentTab, CmagTarget *targetToSelect) {
    popup.shouldBeOpen = true;
    popup.isOpen = false;
    popup.currentTab = currentTab;
    popup.targetToSelect = targetToSelect;
}

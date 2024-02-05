#pragma once

#include "cmag_core/utils/error.h"

struct CmagTarget;
class BrowserState;

class TabChange {
public:
    enum TabSelection {
        Auto,
        TargetGraph,
        ListDir,
        TargetFolder,
        Summary,
    };

    explicit TabChange(BrowserState &browser);

    void change(TabSelection newSelection);
    TabSelection fetch();

    void renderPopup();
    void showPopup(TabSelection currentTab, CmagTarget *targetToSelect);
    bool isPopupShown() const { return popup.shouldBeOpen || popup.isOpen; }

    void setLastDisplayedTab(TabSelection tab);

private:
    BrowserState &browser;
    TabSelection lastDisplayedTab = TabSelection::TargetGraph;
    TabSelection selection = TabSelection::Auto;

    struct {
        bool shouldBeOpen = false;
        bool isOpen = false;
        TabSelection currentTab = TabSelection::Auto;
        CmagTarget *targetToSelect = nullptr;
    } popup;
};

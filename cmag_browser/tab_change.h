#pragma once

#include "cmag_core/utils/error.h"

class TabChange {
public:
    enum TabSelection {
        Auto,
        TargetGraph,
        ListDir,
        TargetFolder,
        Summary,
    };

    void change(TabSelection newSelection);
    TabSelection fetch();

    void renderPopup();
    void showPopup(TabSelection currentTab);

private:
    TabSelection selection = TabSelection::Auto;

    struct {
        bool shouldBeOpen = false;
        bool isOpen = false;
        TabSelection currentTab = TabSelection::Auto;
    } popup;
};
